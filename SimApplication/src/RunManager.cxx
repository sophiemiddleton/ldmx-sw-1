/**
 * @file RunManager.cxx
 * @brief Class providing a Geant4 run manager implementation.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "SimApplication/RunManager.h"

//-------------//
//   ldmx-sw   //
//-------------//
#include "SimApplication/APrimePhysics.h"
#include "SimApplication/DetectorConstruction.h"
#include "SimApplication/GammaPhysics.h"
#include "SimApplication/ParallelWorld.h"
#include "SimApplication/PrimaryGeneratorAction.h"
#include "SimApplication/USteppingAction.h"
#include "SimApplication/UserActionManager.h"
#include "SimApplication/UserEventAction.h"
#include "SimApplication/UserRunAction.h"
#include "SimApplication/UserStackingAction.h"
#include "SimApplication/UserTrackingAction.h"
#include "SimPlugins/PluginManager.h"
#include "SimPlugins/PluginMessenger.h"

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/FrameworkDef.h" 

//------------//
//   Geant4   //
//------------//
#include "FTFP_BERT.hh"
#include "G4GDMLParser.hh"
#include "G4GenericBiasingPhysics.hh"
#include "G4VModularPhysicsList.hh"
#include "G4ParallelWorldPhysics.hh"
#include "G4PhysListFactory.hh"

namespace ldmx {

    RunManager::RunManager(Parameters& parameters) {  

        parameters_ = parameters; 

        pluginManager_ = new PluginManager();
        pluginMessenger_ = new PluginMessenger(pluginManager_);
        
        // Setup messenger for physics list.
        physicsListFactory_ = new G4PhysListFactory;

        // Set whether the ROOT primary generator should use the persisted seed.
        auto rootPrimaryGenUseSeed{parameters.getParameter< int >("rootPrimaryGenUseSeed")}; 
        if (rootPrimaryGenUseSeed < 0) rootPrimaryGenUseSeed = 0; 
        setUseRootSeed(rootPrimaryGenUseSeed); 
    
    }

    RunManager::~RunManager() {
        delete pluginManager_;
        delete pluginMessenger_;
        delete physicsListFactory_; 
    }

    void RunManager::setupPhysics() {

        auto pList{physicsListFactory_->GetReferencePhysList("FTFP_BERT")};
        
        if (isPWEnabled_) {
            std::cout << "[ RunManager ]: Parallel worlds physics list has been registered." << std::endl;
            pList->RegisterPhysics(new G4ParallelWorldPhysics("ldmxParallelWorld"));
        }
        
        pList->RegisterPhysics(new APrimePhysics);
        pList->RegisterPhysics(new GammaPhysics);
       
        if (BiasingMessenger::isBiasingEnabled()) {

            std::cout << "[ RunManager ]: Enabling biasing of particle type " << BiasingMessenger::getParticleType() << std::endl;

            // Instantiate the constructor used when biasing
            G4GenericBiasingPhysics* biasingPhysics = new G4GenericBiasingPhysics();

            // Specify what particles are being biased
            biasingPhysics->Bias(BiasingMessenger::getParticleType());

            // Register the physics constructor to the physics list:
            pList->RegisterPhysics(biasingPhysics);
        }

        this->SetUserInitialization(pList);
    }

    void RunManager::Initialize() {
        
        setupPhysics();

        // The parallel world needs to be registered before the mass world is
        // constructed i.e. before G4RunManager::Initialize() is called. 
        if (isPWEnabled_) {
            std::cout << "[ RunManager ]: Parallel worlds have been enabled." << std::endl;

            G4GDMLParser* pwParser = new G4GDMLParser();
            pwParser->Read(parallelWorldPath_);
            this->getDetectorConstruction()->RegisterParallelWorld(new ParallelWorld(pwParser, "ldmxParallelWorld"));
        }

        G4RunManager::Initialize();

        // Instantiate the primary generator action
        auto primaryGeneratorAction{ new PrimaryGeneratorAction(parameters_) };
        SetUserAction( primaryGeneratorAction );

        // Instantiate action manager
        auto actionManager{UserActionManager::getInstance()}; 

        // Get instances of all G4 actions
        auto actions{actionManager.getActions()};
       
        // Create all user actions
        auto userActions{parameters_.getParameter< std::vector< Class > >("actions")}; 
        std::for_each(userActions.begin(), userActions.end(), 
                [&actionManager](auto& userAction) { 
                    actionManager.createAction(userAction.className_, userAction.instanceName_); 
                }
        );

        // Register all actions with the G4 engine
        for (const auto& [key, act] : actions) {
            std::visit([this](auto&& arg) { this->SetUserAction(arg); }, act); 
        }
    }

    DetectorConstruction* RunManager::getDetectorConstruction() {
        return static_cast<DetectorConstruction*>(this->userDetector); 
    }

} // ldmx 
