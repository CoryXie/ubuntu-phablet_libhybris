#ifndef SHELL_H_
#define SHELL_H_

#include <utils/StrongPointer.h>

namespace mir
{

class ApplicationManager;

class Shell : public android::RefBase
{
  public:
    
    static android::sp<Shell> instance();

  protected:
    Shell(const android::sp<ApplicationManager>& app_manager) : app_manager(app_manager)
    {
    }

    virtual ~Shell()
    {
    }
    
    android::sp<ApplicationManager> app_manager;
};

}

#endif // SHELL_H_
