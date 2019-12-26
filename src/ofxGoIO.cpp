#include "ofxGoIO.h"
//#include "ofLog.h"
//#include "ofMath.h"
//
//#ifdef TARGET_WIN32
//#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
//#include <windows.h>
//#pragma warning(disable: 4996)
//#endif
//#ifdef TARGET_OSX
//#include <Carbon/Carbon.h>
//#endif
//

using namespace std;
//---------------------------------------------------------------------------------------------
ofxGoIO::ofxGoIO()
#ifdef OFX_GO_IO_USE_THREAD
:ofxIOThread(std::bind(&ofxGoIO::threadedFunction, this))
#endif
{
	init();
	state = OFX_GO_IO_STATE_NOT_SETUP;
	
	measurementsInterval = 0.04;//40 milliseconds
	currentMeasurementIndex = 0;
	bHasNewData = false;
	exitListener = ofEvents().exit.newListener([&](ofEventArgs&){
		uninit();
	});
	#ifndef OFX_GO_IO_USE_THREAD
		updateListener = ofEvents().update.newListener([&](ofEventArgs&){
			update();
		});
	#endif
}
ofxGoIO::~ofxGoIO(){
	close();
}
//---------------------------------------------------------------------------------------------
void ofxGoIO::init(){
	if(!isInited()){
		if(GoIO_Init() == 0){
			isInited() = true;
		}
	}else{
		ofLogVerbose("ofxGoIO::init()") << "already inited";
	}
}
//---------------------------------------------------------------------------------------------
void ofxGoIO::uninit(){
	if(isInited()){
		if(GoIO_Uninit() == 0){
			isInited() = false;
		}
	}else{
		ofLogVerbose("ofxGoIO::uninit()") << "cant uninit as it was not inited";
	}
}
//---------------------------------------------------------------------------------------------
bool& ofxGoIO::isInited(){
	static unique_ptr<bool> i =  make_unique<bool>();
	return *i;
}
//---------------------------------------------------------------------------------------------
void addDevices(vector<ofxGoIODevice> & devices, int vid, int pid){
	char deviceName[GOIO_MAX_SIZE_DEVICE_NAME];
	int numDevs = GoIO_UpdateListOfAvailableDevices(vid, pid);
	for(int i = 0; i < numDevs; i ++){
		if(GoIO_GetNthAvailableDeviceName(deviceName, GOIO_MAX_SIZE_DEVICE_NAME, vid, pid, i) == 0){
			devices.push_back(ofxGoIODevice(deviceName, vid, pid));
		}
	}
}

//---------------------------------------------------------------------------------------------
vector<ofxGoIODevice> ofxGoIO::getAvailableDevices(){

	init();
	
	vector<ofxGoIODevice> devices;
	
	addDevices(devices, VERNIER_DEFAULT_VENDOR_ID, SKIP_DEFAULT_PRODUCT_ID);
	addDevices(devices, VERNIER_DEFAULT_VENDOR_ID, USB_DIRECT_TEMP_DEFAULT_PRODUCT_ID);
	addDevices(devices, VERNIER_DEFAULT_VENDOR_ID, CYCLOPS_DEFAULT_PRODUCT_ID);
	addDevices(devices, VERNIER_DEFAULT_VENDOR_ID, MINI_GC_DEFAULT_PRODUCT_ID);

	return devices;
	
}

static std::string getDevicesAsString(const std::vector<ofxGoIODevice>& devices);

//---------------------------------------------------------------------------------------------
string ofxGoIO::getDevicesAsString(const std::vector<ofxGoIODevice>& devices){
	stringstream ss;
	for(auto&d : devices){
		ss << d << endl;
	}
	return ss.str();
}
//---------------------------------------------------------------------------------------------
void ofxGoIO::printDevices(const std::vector<ofxGoIODevice>& devices){
	cout << "--------------------------"<< endl;
	cout << "ofxGoIO available devices:"<< endl;
	cout << getDevicesAsString(devices) << endl;
}
//---------------------------------------------------------------------------------------------
bool ofxGoIO::setup(ofxGoIODevice _device){
	auto devs = getAvailableDevices();
	setState(OFX_GO_IO_STATE_NOT_SETUP);
	if(devs.size() == 0){
		ofLogError("ofxGoIO::setup") << "No devices available";
		return false;
	}
	if(!_device.isSet()){
		if(openDevice(devs[0])){
			setState(OFX_GO_IO_STATE_SETUP);
		}
	}else{
		bool bDeviceFound = false;
		for(auto&d : devs){
			if(d.name == _device.name && d.vendorId == _device.vendorId && d.productId == _device.productId){
				bDeviceFound = true;
				break;
			}
		}
		if(bDeviceFound){
			if(openDevice(_device)){
				setState(OFX_GO_IO_STATE_SETUP);
			}
		}else{
			ofLogWarning("ofxGoIO::setup") << "failed. Device not found.";
		}
	}
	return getState() == OFX_GO_IO_STATE_SETUP;
}
//---------------------------------------------------------------------------------------------
bool ofxGoIO::setup(string deviceName, int vid, int pid){
	return setup({deviceName, vid, pid});
}
//---------------------------------------------------------------------------------------------
bool ofxGoIO::setup(){
	return setup(ofxGoIODevice());
}

//---------------------------------------------------------------------------------------------
bool ofxGoIO::openDevice( ofxGoIODevice device){
	
	
	if (handle)
	{
		char openDeviceName[GOIO_MAX_SIZE_DEVICE_NAME];
		int openVendorId, openProductId;
		GoIO_Sensor_GetOpenDeviceName(handle, openDeviceName, GOIO_MAX_SIZE_DEVICE_NAME, &openVendorId, &openProductId);

		if (0 != device.name.compare(string(openDeviceName, GOIO_MAX_SIZE_DEVICE_NAME)))
		{
			//Close the open device if it does not match the new one.
			close();
		}else{
			ofLogVerbose( "ofxGoIO::openDevice") << "device " << device << "is already opened";
			return true;
		}
	}
	if (!handle){
		if(!device.isSet()){
			auto devs = getAvailableDevices();
			if(devs.size()){
				device = devs[0];
			}else{
				ofLogVerbose( "ofxGoIO::openDevice") << "could not auto assign a device as there are non available";
				return false;
			}
		}
		handle = GoIO_Sensor_Open(device.name.c_str(), device.vendorId, device.productId, 0);
	}
	if (handle)
	{
		this->currentDevice = device;
		
		getDeviceIdAndLongName(this->currentDevice);
		
		cout << "Successfully opened device:\n" << this->currentDevice << endl;
	
		cout << getCurrentCalibrationFromDevice() << endl;
		
	}else{
		ofLogError("ofxGoIO::openDevice") << "failed to open device " << device;
	}
	
	return (handle != NULL);
}
//---------------------------------------------------------------------------------------------
bool ofxGoIO::openDevice(string deviceName, int vid, int pid){
	return openDevice({deviceName, vid, pid});
}
//---------------------------------------------------------------------------------------------
bool ofxGoIO::getDeviceIdAndLongName(ofxGoIODevice& dev){
	
	bool ret = false;
	ret |= (GoIO_Sensor_DDSMem_GetSensorNumber(handle, &dev.id, 0, 0) == 0);
	
	char tmpstring[50];
	if(GoIO_Sensor_DDSMem_GetLongName(handle, tmpstring, sizeof(tmpstring)) == 0){
		this->currentDevice.longName = tmpstring;
		ret = true;
	}
	return ret;
}
//---------------------------------------------------------------------------------------------
void ofxGoIO::close(){
	if(handle != NULL){
		GoIO_Sensor_Close(handle);
		handle = NULL;
	}
}
//---------------------------------------------------------------------------------------------
bool ofxGoIO::isSetup(){
	return state != OFX_GO_IO_STATE_NOT_SETUP;
}
//---------------------------------------------------------------------------------------------
void ofxGoIO::clear(){
	if(handle != NULL){
		GoIO_Sensor_ClearIO(handle);
	}
}
//---------------------------------------------------------------------------------------------
double ofxGoIO::getMeasurementTickInSeconds(){
	auto ret = GoIO_Sensor_GetMeasurementTickInSeconds(	handle );
	if(ret < 0){
		ofLogWarning("ofxGoIO::getMeasurementTickInSeconds") << "failed";
	}
	return ret;
}
//---------------------------------------------------------------------------------------------
double ofxGoIO::getMinimumMeasurementPeriod(){
	auto ret = GoIO_Sensor_GetMinimumMeasurementPeriod(	handle );
	if(ret < 0){
		ofLogWarning("ofxGoIO::getMinimumMeasurementPeriod") << "failed";
	}
	return ret;
}
//---------------------------------------------------------------------------------------------
double ofxGoIO::getMaximumMeasurementPeriod(){
	auto ret = GoIO_Sensor_GetMaximumMeasurementPeriod(	handle );
	if(ret < 0){
		ofLogWarning("ofxGoIO::getMaximumMeasurementPeriod") << "failed";
	}
	return ret;
}
//---------------------------------------------------------------------------------------------
bool ofxGoIO::setMeasurementPeriod( double desiredPeriod, int timeoutMs){
	
	auto prevState = getState();
	if(prevState == OFX_GO_IO_STATE_MEASURING){
		stopMeasurements();
	}
	setState(OFX_GO_IO_STATE_SETTING_INTERVAL);
	bool bSuccess = GoIO_Sensor_SetMeasurementPeriod( handle, desiredPeriod, timeoutMs) == 0;
	if(bSuccess){
		getMeasurementPeriod(timeoutMs);
	}else{
		ofLogWarning("ofxGoIO::setMeasurementPeriod") << "failed";
	}
	if(prevState == OFX_GO_IO_STATE_MEASURING){
		startMeasurements();
	}else{
		setState(prevState);
	}
	return bSuccess;
}
//---------------------------------------------------------------------------------------------
double ofxGoIO::getMeasurementPeriod( int timeoutMs){
	auto ret = GoIO_Sensor_GetMeasurementPeriod( handle, timeoutMs);
	if( ofIsFloatEqual(ret, 1000000.0)){
		ofLogWarning("ofxGoIO::getMeasurementPeriod") << "failed";
	}else{
		measurementsInterval = ret;
	}
	return ret;
}
//---------------------------------------------------------------------------------------------
int ofxGoIO::getNumMeasurementsAvailable(){
	return GoIO_Sensor_GetNumMeasurementsAvailable(	handle );
}
//---------------------------------------------------------------------------------------------
void ofxGoIO::calibrate( size_t numSamples){
	
	calibrationData.setup(numSamples);
	
	stateBeforeCalibration = getState();
	setState(OFX_GO_IO_STATE_CALIBRATING);

}
//---------------------------------------------------------------------------------------------
void ofxGoIO::updateCalibration(){
	if(getState() == OFX_GO_IO_STATE_CALIBRATING){
		size_t m = getNumMeasurementsAvailable();
		if(m > 0){
			if(calibrationData.currentSample < calibrationData.numSamples){
				auto n = GoIO_Sensor_ReadRawMeasurements(handle, &calibrationData.rawMeasurements[calibrationData.currentSample], std::min(m, calibrationData.numSamples - calibrationData.currentSample));
				calibrationData.currentSample += n;
			}else{
				auto cal = getCurrentCalibrationFromDevice();
				calibrationData.process(handle);
				
				cout << "Calibration Ended. Average measurement: " << calibrationData.averageCalbMeasurement << currentDeviceCalibration.units << endl;
				
				setState(stateBeforeCalibration);
				ofNotifyEvent(calibrationEndEvent, cal,this);
				
			}
		}
	}
}
//---------------------------------------------------------------------------------------------
void ofxGoIO::updateMeasurements(){
	if(getState() == OFX_GO_IO_STATE_MEASURING){
		size_t m = getNumMeasurementsAvailable();
		if(m){
			
			
			vector<int> tempData (m);
			auto n = GoIO_Sensor_ReadRawMeasurements(handle,
													 tempData.data(),
													 m);
			auto t = ofGetElapsedTimef();
//			for(size_t i =0; i < lastMeasurement.data.size(); i ++){
//				lastMeasurement.data
//			}
//			lastMeasurement.sampleTimeInterval =
//			measurementsInterval;
			if(n != m){
				ofLogError("ofxGoIO::updateMeasurements()") << "did not read all available measurements";
			}
			if(n){
				bHasNewData = true;
#ifdef OFX_GO_IO_USE_THREAD
				unique_lock<std::mutex> lck(dataMutex);
#endif
				lastMeasurement.resize(n);
				for(size_t i = 0; i < n; i++){
					lastMeasurement[i].rawData = tempData[i];
					measurementsBuffer[currentMeasurementIndex].rawData =  tempData[i];
					measurementsBuffer[currentMeasurementIndex].aquisitionTime = lastMeasurement[i].aquisitionTime = t - measurementsInterval * (n -1 -i) ;
					++currentMeasurementIndex %= measurementsBuffer.size();
				}
				
			}
			ofNotifyEvent(newMeasurementEvent, lastMeasurement, this);
		}
	}
}
 //---------------------------------------------------------------------------------------------
bool ofxGoIO::startMeasurements(){
	return startMeasurements(OFX_GO_IO_DEFAULT_TIMEOUT);
}
//---------------------------------------------------------------------------------------------
bool ofxGoIO::startMeasurements(int timeoutMs){
	if(getState() != OFX_GO_IO_STATE_MEASURING){
		if(setMeasurementPeriod(measurementsInterval, timeoutMs)){
			bool bRet = GoIO_Sensor_SendCmdAndGetResponse(handle, SKIP_CMD_ID_START_MEASUREMENTS, NULL, 0, NULL, NULL, timeoutMs) == 0;
			if(bRet){
				currentMeasurementIndex = 0;
				measurementsBuffer.resize(OFX_GO_IO_DEFAULT_NUM_MEASUREMENTS);
				
				setState(OFX_GO_IO_STATE_MEASURING);
				return true;
			}
		}
	}
	ofLogError("ofxGoIO::startMeasurements") << "failed";
	return false;
}
//---------------------------------------------------------------------------------------------
bool ofxGoIO::stopMeasurements(){
	return stopMeasurements(OFX_GO_IO_DEFAULT_TIMEOUT);
}
//---------------------------------------------------------------------------------------------
bool ofxGoIO::stopMeasurements(int timeoutMs){
	if(getState() == OFX_GO_IO_STATE_MEASURING){
		if(GoIO_Sensor_SendCmdAndGetResponse(handle, SKIP_CMD_ID_STOP_MEASUREMENTS, NULL, 0, NULL, NULL, timeoutMs) == 0){
			setState(OFX_GO_IO_STATE_SETUP);
			return true;
		}
	}
	ofLogError("ofxGoIO::stopMeasurements") << "failed";
	return false;
}
//---------------------------------------------------------------------------------------------
void ofxGoIO::update(){
	if(getState() == OFX_GO_IO_STATE_CALIBRATING){
		updateCalibration();
	}else if(getState() == OFX_GO_IO_STATE_MEASURING){
		updateMeasurements();
	}
}
#ifdef OFX_GO_IO_USE_THREAD
//---------------------------------------------------------------------------------------------
bool ofxGoIO::shouldRepeatWithDelay(uint64_t& delay) {
	delay = measurementsInterval * 1000;
	return true;
}
//---------------------------------------------------------------------------------------------
void ofxGoIO::threadedFunction(){
	update();
}
#endif
//---------------------------------------------------------------------------------------------
const ofxGoIODeviceCalibrationProfile& ofxGoIO::getCurrentCalibrationFromDevice(){

	if(GoIO_Sensor_DDSMem_GetActiveCalPage(handle, &currentDeviceCalibration.calPageIndex)== 0){
		char tmpstring[50];
		if(GoIO_Sensor_DDSMem_GetCalPage(handle,
										 currentDeviceCalibration.calPageIndex,
										 &currentDeviceCalibration.coeff[0],
										 &currentDeviceCalibration.coeff[1],
										 &currentDeviceCalibration.coeff[2],
										 tmpstring, sizeof(tmpstring)) == 0)
		{
			currentDeviceCalibration.units = tmpstring;
		}
		GoIO_Sensor_DDSMem_GetCalibrationEquation(handle, &currentDeviceCalibration.equationType);
	}
	return currentDeviceCalibration;
}
//---------------------------------------------------------------------------------------------
ofxGoIO::State ofxGoIO::getState(){
	return state;
}
//---------------------------------------------------------------------------------------------
void ofxGoIO::setState(ofxGoIO::State newState){
	if(state != newState){
		state = newState;
		ofLogVerbose("ofxGoIO::setState") << " New state: \"" << stateToString(newState) << "\"";
		// do whatever needed for the new state
#ifdef OFX_GO_IO_USE_THREAD
		if(!isRunning() && (state == OFX_GO_IO_STATE_MEASURING || state == OFX_GO_IO_STATE_CALIBRATING)){
			 start();
		}
#endif
		
	}else{
		ofLogVerbose("ofxGoIO::setState") << "State \"" << stateToString(newState) << "\" already set";
	}
}
//---------------------------------------------------------------------------------------------
const ofxGoIODevice& ofxGoIO::getCurrentDevice(){
	return currentDevice;
}
//---------------------------------------------------------------------------------------------
std::string ofxGoIO::stateToString(ofxGoIO::State s){
	switch(s){
		case OFX_GO_IO_STATE_NOT_SETUP: return "Not Setup";
		case OFX_GO_IO_STATE_SETUP: return "Setup";
		case OFX_GO_IO_STATE_MEASURING: return "Measuring";
		case OFX_GO_IO_STATE_SETTING_INTERVAL: return "Setting Interval";
		case OFX_GO_IO_STATE_CALIBRATING: return "Calibrating";
	}
	return "";
}
//---------------------------------------------------------------------------------------------
GOIO_SENSOR_HANDLE ofxGoIO::getDeviceHandle(){
	return handle;
}

//---------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------
void ofxGoIODeviceCalibrationData::setup(size_t _numSamples){
	numSamples = _numSamples;
	rawMeasurements.resize(_numSamples);
	volts.resize(_numSamples);
	calbMeasurements.resize(_numSamples);
	averageCalbMeasurement = 0;
	currentSample = 0;
}
//---------------------------------------------------------------------------------------------
size_t ofxGoIODeviceCalibrationData::size() {
	return numSamples;
}
//---------------------------------------------------------------------------------------------
void ofxGoIODeviceCalibrationData::clear(){
	numSamples = 0;
	rawMeasurements.clear();
	volts.clear();
	calbMeasurements.clear();
}
//---------------------------------------------------------------------------------------------
bool ofxGoIODeviceCalibrationData::process(GOIO_SENSOR_HANDLE handle){
	if(handle && numSamples > 1){
		averageCalbMeasurement = 0;
		for (size_t i = 0; i < numSamples; i++){
			volts[i] = GoIO_Sensor_ConvertToVoltage(handle, rawMeasurements[i]);
			calbMeasurements[i] = GoIO_Sensor_CalibrateData(handle, volts[i]);
			averageCalbMeasurement += calbMeasurements[i];
		}
		averageCalbMeasurement = averageCalbMeasurement/numSamples;
		return true;
	}
	return false;
}
//---------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------
std::ostream& operator<< (std::ostream& os, const ofxGoIODeviceCalibrationProfile& profile){
	os << "ofxGoIO Device Calibration Profile:" << std::endl;
	os << "    Page Index: " << (int)profile.calPageIndex << std::endl;
	os << "    Coefficients: " << profile.coeff[0] << ", " << profile.coeff[1] << ", " << profile.coeff[2] << std::endl;
	os << "    Equation type: " << (int)profile.equationType;
	if (profile.units.size()){
		os  << std::endl << "    units: " << profile.units ;
	}
	return os;
}
//---------------------------------------------------------------------------------------------
std::ostream& operator<<(std::ostream& os, const ofxGoIOMeasurement& data){
	os << "Measurement: " << std::endl;
	os << "     Num Samples:  " << data.size();
	for(auto&d: data){
		os  << std::endl << "     " << d.rawData  << " Aquisition time:  " << d.aquisitionTime;
	}
	
	return os;
}
//---------------------------------------------------------------------------------------------
std::ostream& operator<<(std::ostream& os, const ofxGoIODevice& dev){
	os << "Name: " << dev.name << std::endl;
	os << "Vendor Id: " << dev.vendorId << std::endl;
	os << "Product Id: " << dev.productId << std::endl;
	os << "Device Id: " << (int) dev.id ;
	if(dev.longName.size()){
		os  << std::endl << "Long Name: " << dev.longName;
	}
	return os;
}


	
