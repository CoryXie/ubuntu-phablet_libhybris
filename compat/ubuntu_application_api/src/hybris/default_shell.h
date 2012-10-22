#ifndef DEFAULT_SHELL_H_
#define DEFAULT_SHELL_H_

#include "shell.h"

namespace mir
{

class DefaultShell : public Shell
{
  public:
    
    static android::sp<Shell> instance(const android::sp<ApplicationManager>& app_manager)
    {
        static android::sp<Shell> shell(new DefaultShell(app_manager));
        return shell;
    }

  protected:
    DefaultShell(const android::sp<ApplicationManager>& app_manager) : Shell(app_manager)
    {

    }
};

}

#endif // DEFAULT_SHELL_H_
