#include <XnUSB.h>
#include <cstdio>
#include "cinder/app/AppBasic.h"
#include "cinder/Utilities.h"

using namespace ci;
using namespace ci::app;
using namespace std;

//#ifdef _WIN32
//#include <Windows.h>
//
//
//void pause_ ()
//{
//    Sleep(1000);
//}
//#else
//#include <unistd.h>
//void pause_ ()
//{
//    //ci::sleep(1000);
//}
//#endif

/**
 * Class to control Kinect's motor.
 */
class KinectMotors
{
public:
    enum { MaxDevs = 16 };
    
public:
    KinectMotors();
    virtual ~KinectMotors();
    
    /**
     * Open device.
     * @return true if succeeded, false - overwise
     */
    bool Open();
    
    /**
     * Close device.
     */
    void Close();
    
    /**
     * Move motor up or down to specified angle value.
     * @param angle angle value
     * @return true if succeeded, false - overwise
     */
    bool Move(int angle);
    
private:
    XN_USB_DEV_HANDLE m_devs[MaxDevs];
    XnUInt32 m_num;
    bool m_isOpen;
};