
#include <main.h>

static Mutex ExitLock;
static bool DoExit = false;

static void print_help(FILE *fp, char *app)
{
    fprintf(fp, "Usage: %s <options>\n", app);
    fprintf(fp, "\n");
    fprintf(fp, " -h --help             Print this help\n");
    fprintf(fp, " -d --debug            Switch on debug level logging\n");
    fprintf(fp, "\n");
}

class SigHandler : public ISignalHandler
{
    public:
        SigHandler() :
            m_manager(NULL)
        {

        }

        ~SigHandler()
        {

        }
        
        void SetMonitorManager(MonitorManager *manager)
        {
            m_manager = manager;
        }

        void SigHUP(const siginfo_t *info)
        {
            LogDebug("SigHandler::SigHUP");
            LoggerRotate();
        }

        void SigTerm(const siginfo_t *info)
        {
            LogDebug("SigHandler::SigTerm");
            ScopedLock lock(&ExitLock);
            DoExit = true;
            ExitLock.WakeUp();
        }

        void SigUser1(const siginfo_t *info)
        {
            LogDebug("SigHandler::SigUser1");
            if (m_manager)
                m_manager->Dump();
        }

        void SigUser2(const siginfo_t *info)
        {
            LogDebug("SigHandler::SigUser2");
            if (m_manager)
                m_manager->Reset();
        }

        void SigPipe(const siginfo_t *info)
        {
            //Just ignored
        }

    private:
        MonitorManager *m_manager;
};

class Service : public IServerHandler
{
    private:
        MonitorManager *Manager;
        std::map<std::string, std::function<int(IServerConnection *Connection, Json::Value &request, Json::Value &response)> > m_Requests;

    public:
        Service(MonitorManager *MM)
        {
            Manager = MM;
        }
    
        ~Service()
        {
    
        }
    
        void OnPreNewConnection()
        {
            LogDebug("Service::OnPreNewConnection");
        }
    
        void OnPostNewConnection(IServerConnection *Connection)
        {
            LogDebug("Service::OnPostNewConnection");
        }
    
        void OnDisconnect(IServerConnection *Connection)
        {
            LogDebug("Service::OnDisconnect");
        }
    
        int OnRequest(IServerConnection *Connection, Json::Value &request, Json::Value &response)
        {
            if (request.isMember("action") && request["action"].isString()) {
                //LOGGER << LOGGER_DEBUG << "OnRequest: " << request["action"] << std::endl;

                const std::string action = request["action"].asString();
                //Lookup out request to save string compares.
                auto func = m_Requests.find(action);
                if (func != m_Requests.end()) {
                    int ret = 0;
                    PerfCounter Perf(action);
                    try {
                        ret = func->second(Connection, request, response);
                    } catch(std::exception &ex) {
                        LogError("Exception On Request: %s Error: %s", action.c_str(), ex.what());
                        throw(std::runtime_error(ex.what()));
                    }
                    return ret;
                } else {
                    LOGGER << LOGGER_ERR << "Unknown Request: " << action << std::endl;
                    return -EINVAL;
                }
            }
            return -EINVAL;
        }
    
        int OnCommand(IServerConnection *Connection, Json::Value &request)
        {
            LogDebug("OnCommand");
            return -EINVAL;
        }
    
        void OnBadLine(IServerConnection *Connection, const std::string *line)
        {
            LogError("OnBadLine: %s\n", line->c_str());
        }
};

int main(int argc, char **argv)
{
    std::string LocSocket = "/tmp/ipdupdetect";
    std::unique_ptr<PIDFile> PidFile;
    std::string LocPidFile = "";
    const char *opts = "hdp:";
    int longindex = 0;
    int c = 0;
    int debug = 0;
    struct option loptions[]
    {
        {"help", 0, 0, 'h'},
        {"pid", 1, 0, 'p'},
        {"debug", 0, 0, 'd'},
        {0, 0, 0, 0}
    };
    
    while( (c = getopt_long(argc, argv, opts, loptions, &longindex)) >= 0)
    {
        switch(c)
        {
            case 'd':
                debug = 1;
                break;
            case 'h':
                print_help(stdout, argv[0]);
                exit(EXIT_SUCCESS);
                break;
            case 'p':
                LocPidFile = optarg;
                break;
            default:
                break;
        }
    }
    
    //Add Logging
    if (isatty(fileno(stdout)) == 1)
    {
        std::shared_ptr<ILogger> tmp = std::make_shared<LogStdoutColor>();
        LogManager::Add(tmp);
    } else {
        std::shared_ptr<ILogger> tmp = std::make_shared<LogStdout>();
        LogManager::Add(tmp);
    }

    if (debug)
    {
        LogManager::SetLevel(LOGGER_DEBUG);
    }
    else
    {
        LogManager::SetLevel(LOGGER_INFO);
    }
    
    if (LocPidFile != "")
    {
        PidFile.reset(new PIDFile(LocPidFile));
        if (PidFile->Create() == false)
        {
            LogCritical("Cannot Create PID file '%s'", LocPidFile.c_str());
            exit(EXIT_FAILURE);
        }
        LogInfo("Created PID file '%s'", LocPidFile.c_str());
    }

    SigHandler SHandler;
    SignalHandler Signals = SignalHandler(&SHandler);
    Signals.Block();

    do
    {
        ServerManager *SrvManager = NULL;
        MonitorManager MManager;
        Service *Srv = new Service(&MManager); //Bind instance of MonitorManager to the ServiceProxy
        struct timespec timeout = {60, 0}; //Timeout to reprocess interface list
        ScopedLock lock(&ExitLock);
        SHandler.SetMonitorManager(&MManager); //Bind MonitorManager to signal handler proxy

        
        ServerUnixPolled Unix(LocSocket); //Create a suitable socket
        SrvManager = new ServerManager(Srv); //Create a new server instance
        SrvManager->ServerAdd(&Unix); //Bind our Service proxy to the socket instance

        Signals.UnBlock();
        while(DoExit == false)
        {
            MManager.Scan();
            MManager.Purge();
            ExitLock.Wait(&timeout);
        }
        lock.Unlock(); //Required to prevent hang in signals
        
        SrvManager->ServerRemove(&Unix);
        delete Srv;
        delete SrvManager;
        
        Signals.Block();
        SHandler.SetMonitorManager(NULL);
        Signals.UnBlock();
        
    } while(0);


    return 0;
}


