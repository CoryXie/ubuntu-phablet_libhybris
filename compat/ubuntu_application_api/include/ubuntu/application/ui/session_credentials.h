#ifndef UBUNTU_APPLICATION_UI_SESSION_CREDENTIALS_H_
#define UBUNTU_APPLICATION_UI_SESSION_CREDENTIALS_H_

namespace ubuntu
{
namespace application
{
namespace ui
{
struct SessionCredentials
{
    enum { max_application_name_length = 512 };
    char application_name[max_application_name_length];
};
}
}
}

#endif // UBUNTU_APPLICATION_UI_SESSION_CREDENTIALS_H_
