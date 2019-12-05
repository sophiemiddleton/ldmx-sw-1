
#include "Framework/NtupleManager.h" 

namespace ldmx {

    NtupleManager* NtupleManager::instance_ = nullptr;

    NtupleManager::NtupleManager() { } 

    NtupleManager* NtupleManager::getInstance() {
        
        // Create an instance of the NtupleManager if needed
        if (!instance_) instance_ = new NtupleManager; 
        
        return instance_; 
    }

    void NtupleManager::create(const std::string& name) {

        // Check if a tree with named 'name' has already been created.  If so
        // throw an exception. 
        if (trees_.count(name) != 0) 
            EXCEPTION_RAISE(
                    "NtupleManager", 
                    "A tree with name " + name + " has already been created." 
            ); 

        // Create a tree with the given name and add it to the list of trees.
        auto tree{new TTree{name.c_str(), name.c_str()}};
        trees_[name] = tree;       
    }

    void NtupleManager::fill() {
        
        // Check for the existence of the tree with the name tname. If it
        // doesn't exists, throw an exception
        if (trees_.empty()) return;
        
        // Loop over all the trees and fill them
        for (const auto& element : trees_) { 
            element.second->Fill(); 
        }
    }

    void NtupleManager::clear() { 
        
        if (variables_.empty()) return; 

        for (const auto& element : variables_) { 
            variables_[element.first] = -9999; 
        }
    }
}
