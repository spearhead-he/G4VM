#include "UserActionInitialization.hh"
#include "PrimaryGeneratorAction.hh"

UserActionInitialization::UserActionInitialization() {
  ;
}
UserActionInitialization::~UserActionInitialization() {
  ;
}

void UserActionInitialization::Build() const {
  SetUserAction(new PrimaryGeneratorAction);
}
  
void UserActionInitialization::BuildForMaster() const {
}

