
#include "CaptureDevice.h"

#include "Error.h"

#include "logging.h"
#include "utils.h"
#include "serialization.h"

using namespace tcam;


CaptureDevice::CaptureDevice ()
    : pipeline(nullptr), property_handler(nullptr), device(nullptr)
{}


CaptureDevice::~CaptureDevice ()
{
    if (isDeviceOpen())
        closeDevice();
}


bool CaptureDevice::load_configuration (const std::string& filename)
{
    resetError();

    if (!isDeviceOpen())
    {
        return false;
    }

    auto vec = property_handler->get_properties();

    return load_xml_description(filename,
                                open_device,
                                active_format,
                                vec);
}


bool CaptureDevice::save_configuration (const std::string& filename)
{
    resetError();

    if (!isDeviceOpen())
    {
        return false;
    }

    return save_xml_description(filename,
                                open_device,
                                device->getActiveVideoFormat(),
                                property_handler->get_properties());
}


bool CaptureDevice::openDevice (const DeviceInfo& device_desc)
{
    resetError();

    if (isDeviceOpen())
    {
        bool ret = closeDevice();
        if (ret == false)
        {
            tcam_log(TCAM_LOG_ERROR, "Unable to close previous device.");
            // setError(Error("A device is already open", EPERM));
            return false;
        }
    }

    open_device = device_desc;

    device = openDeviceInterface(open_device);

    if (device == nullptr)
    {
        return false;
    }

    property_handler = std::make_shared<PropertyHandler>();

    property_handler->set_properties(device->getProperties(), pipeline->getFilterProperties());

    return true;
}


bool CaptureDevice::isDeviceOpen () const
{
    resetError();
    if (device != nullptr)
    {
        return true;
    }

    return false;
}


DeviceInfo CaptureDevice::getDevice () const
{
    return this->open_device;
}


bool CaptureDevice::closeDevice ()
{
    std::string name = open_device.getName();

    pipeline->destroyPipeline();

    open_device = DeviceInfo ();
    device.reset();
    property_handler = nullptr;

    tcam_log(TCAM_LOG_INFO, "Closed device %s.", name.c_str());

    return true;
}


std::vector<Property> CaptureDevice::getAvailableProperties () const
{
    resetError();
    if (!isDeviceOpen())
    {
        return std::vector<Property>();
    }

    std::vector<Property> props;

    for ( const auto& p : property_handler->get_properties())
    {
        props.push_back(*p);
    }

    return props;
}


std::vector<VideoFormatDescription> CaptureDevice::getAvailableVideoFormats () const
{
    resetError();
    if (!isDeviceOpen())
    {
        return std::vector<VideoFormatDescription>();
    }

    return pipeline->getAvailableVideoFormats();
}


bool CaptureDevice::setVideoFormat (const VideoFormat& new_format)
{
    resetError();
    if (!isDeviceOpen())
    {
        return false;
    }

    pipeline->setVideoFormat(new_format);

    return this->device->setVideoFormat(new_format);
}


VideoFormat CaptureDevice::getActiveVideoFormat () const
{
    resetError();
    if(!isDeviceOpen())
    {
        return VideoFormat();
    }

    return device->getActiveVideoFormat();
}


bool CaptureDevice::startStream (std::shared_ptr<SinkInterface> sink)
{
    resetError();
    if (!isDeviceOpen())
    {
        return false;
    }
    pipeline->setSink(sink);

    return pipeline->setStatus(TCAM_PIPELINE_PLAYING);
}


bool CaptureDevice::stopStream ()
{
    resetError();
    if (!isDeviceOpen())
    {
        return false;
    }

    return pipeline->setStatus(TCAM_PIPELINE_STOPPED);
}
