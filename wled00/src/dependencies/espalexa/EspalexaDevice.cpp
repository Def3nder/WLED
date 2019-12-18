//EspalexaDevice Class

#include "EspalexaDevice.h"

EspalexaDevice::EspalexaDevice(){}

EspalexaDevice::EspalexaDevice(String deviceName, BrightnessCallbackFunction gnCallback, uint8_t initialValue) { //constructor for dimmable device
  
  _deviceName = deviceName;
  _callback = gnCallback;
  _val = initialValue;
  _val_last = _val;
  _type = EspalexaDeviceType::dimmable;
}

EspalexaDevice::EspalexaDevice(String deviceName, ColorCallbackFunction gnCallback, uint8_t initialValue) { //constructor for color device
  
  _deviceName = deviceName;
  _callbackCol = gnCallback;
  _val = initialValue;
  _val_last = _val;
  _type = EspalexaDeviceType::extendedcolor;
}

EspalexaDevice::EspalexaDevice(String deviceName, DeviceCallbackFunction gnCallback, EspalexaDeviceType t, uint8_t initialValue) { //constructor for general device
  
  _deviceName = deviceName;
  _callbackDev = gnCallback;
  _type = t;
  if (t == EspalexaDeviceType::onoff) _type = EspalexaDeviceType::dimmable; //on/off is broken, so make dimmable device instead
  _val = initialValue;
  _val_last = _val;
}

EspalexaDevice::~EspalexaDevice(){/*nothing to destruct*/}

uint8_t EspalexaDevice::getId()
{
  return _id;
}

EspalexaColorMode EspalexaDevice::getColorMode()
{
  return _mode;
}

EspalexaDeviceType EspalexaDevice::getType()
{
  return _type;
}

String EspalexaDevice::getName()
{
  return _deviceName;
}

EspalexaDeviceProperty EspalexaDevice::getLastChangedProperty()
{
  return _changed;
}

uint8_t EspalexaDevice::getValue()
{
  return _val;
}

uint8_t EspalexaDevice::getPercent()
{
  uint16_t perc = _val * 100;
  return perc / 255;
}

uint8_t EspalexaDevice::getDegrees()
{
  return getPercent();
}

uint16_t EspalexaDevice::getHue()
{
  return _hue;
}

uint8_t EspalexaDevice::getSat()
{
  return _sat;
}

float EspalexaDevice::getX()
{
  return _x;
}

float EspalexaDevice::getY()
{
  return _y;
}

float EspalexaDevice::minf (float v, float w)
{
  if (w > v) return v;
  return w;
}

float EspalexaDevice::maxf (float v, float w)
{
  if (w > v) return w;
  return v;
}

uint16_t EspalexaDevice::getCt()
{
  if (_ct == 0) return 500;
  return _ct;
}

uint32_t EspalexaDevice::getKelvin()
{
  if (_ct == 0) return 2000;
  return 1000000/_ct;
}

uint32_t EspalexaDevice::getRGB()
{
  if (_rgb != 0) return _rgb; //color has not changed
  byte rgb[4]{0, 0, 0, 0}; 
  float r, g, b, w;
  
  if (_mode == EspalexaColorMode::none) return 0;

  if (_mode == EspalexaColorMode::ct)
  {
    //TODO tweak a bit to match hue lamp characteristics
    //based on https://gist.github.com/paulkaplan/5184275
    float temp = 10000/ _ct; //kelvins = 1,000,000/mired (and that /100)
    float r, g, b;

    // Cold white to warm white receiving from Alexa: _ct = 199, 234, 284, 350, 383 (from cold white to warm white)
    switch (_ct) {
      case 199: rgb[0]=255,rgb[1]=255,rgb[2]=255;rgb[3]=255;break;
      case 234: rgb[0]=127,rgb[1]=127,rgb[2]=127;rgb[3]=255;break;
      case 284: rgb[0]=0,rgb[1]=0,rgb[2]=0;rgb[3]=255;break;
      case 350: rgb[0]=130,rgb[1]=90,rgb[2]=0;rgb[3]=255;break;
      case 383: rgb[0]=255,rgb[1]=153,rgb[2]=0;rgb[3]=255;break;
      default: {
 
    if( temp <= 66 ){ 
      r = 255; 
      g = temp;
      g = 99.470802 * log(g) - 161.119568;
      if( temp <= 19){
          b = 0;
      } else {
          b = temp-10;
          b = 138.517731 * log(b) - 305.044793;
      }
    }
    
    rgb[0] = (byte)constrain(r,0.1,255.1);
    rgb[1] = (byte)constrain(g,0.1,255.1);
    rgb[2] = (byte)constrain(b,0.1,255.1);

    }
  } else if (_mode == EspalexaColorMode::hs)
  {
    float h = ((float)_hue)/65535.0;
    float s = ((float)_sat)/255.0;
    byte i = floor(h*6);
    float f = h * 6-i;
    float p = 255 * (1-s);
    float q = 255 * (1-f*s);
    float t = 255 * (1-(1-f)*s);
    switch (i%6) {
      case 0: rgb[0]=255,rgb[1]=t,rgb[2]=p;break;
      case 1: rgb[0]=q,rgb[1]=255,rgb[2]=p;break;
      case 2: rgb[0]=p,rgb[1]=255,rgb[2]=t;break;
      case 3: rgb[0]=p,rgb[1]=q,rgb[2]=255;break;
      case 4: rgb[0]=t,rgb[1]=p,rgb[2]=255;break;
      case 5: rgb[0]=255,rgb[1]=p,rgb[2]=q;
    }
    RGBtoRGBW(rgb);
  } else if (_mode == EspalexaColorMode::xy)
  {
    //Source: https://www.developers.meethue.com/documentation/color-conversions-rgb-xy
    float z = 1.0f - _x - _y;
    float X = (1.0f / _y) * _x;
    float Z = (1.0f / _y) * z;
    float r = (int)255*(X * 1.656492f - 0.354851f - Z * 0.255038f);
    float g = (int)255*(-X * 0.707196f + 1.655397f + Z * 0.036152f);
    float b = (int)255*(X * 0.051713f - 0.121364f + Z * 1.011530f);
    if (r > b && r > g && r > 1.0f) {
      // red is too big
      g = g / r;
      b = b / r;
      r = 1.0f;
    } else if (g > b && g > r && g > 1.0f) {
      // green is too big
      r = r / g;
      b = b / g;
      g = 1.0f;
    } else if (b > r && b > g && b > 1.0f) {
      // blue is too big
      r = r / b;
      g = g / b;
      b = 1.0f;
    }
    // Apply gamma correction
    r = r <= 0.0031308f ? 12.92f * r : (1.0f + 0.055f) * pow(r, (1.0f / 2.4f)) - 0.055f;
    g = g <= 0.0031308f ? 12.92f * g : (1.0f + 0.055f) * pow(g, (1.0f / 2.4f)) - 0.055f;
    b = b <= 0.0031308f ? 12.92f * b : (1.0f + 0.055f) * pow(b, (1.0f / 2.4f)) - 0.055f;

    if (r > b && r > g) {
      // red is biggest
      if (r > 1.0f) {
        g = g / r;
        b = b / r;
        r = 1.0f;
      }
    } else if (g > b && g > r) {
      // green is biggest
      if (g > 1.0f) {
        r = r / g;
        b = b / g;
        g = 1.0f;
      }
    } else if (b > r && b > g) {
      // blue is biggest
      if (b > 1.0f) {
        r = r / b;
        g = g / b;
        b = 1.0f;
      }
    }
    rgb[0] = 255.0*r;
    rgb[1] = 255.0*g;
    rgb[2] = 255.0*b;
    RGBtoRGBW(rgb);
  }
  _rgb = ((rgb[3] << 24) | (rgb[0] << 16) | (rgb[1] << 8) | (rgb[2]));  //white value is only >0 if Alexa did provide a CT value
  return _rgb;
}

uint8_t EspalexaDevice::getW()
{
  return (getRGB() >> 24) & 0xFF;
}

uint8_t EspalexaDevice::getR()
{
  return (getRGB() >> 16) & 0xFF;
}

uint8_t EspalexaDevice::getG()
{
  return (getRGB() >> 8) & 0xFF;
}

uint8_t EspalexaDevice::getB()
{
  return getRGB() & 0xFF;
}

uint8_t EspalexaDevice::getLastValue()
{
  if (_val_last == 0) return 255;
  return _val_last;
}

void EspalexaDevice::setPropertyChanged(EspalexaDeviceProperty p)
{
  _changed = p;
}

void EspalexaDevice::setId(uint8_t id)
{
  _id = id;
}

//you need to re-discover the device for the Alexa name to change
void EspalexaDevice::setName(String name)
{
  _deviceName = name;
}

void EspalexaDevice::setValue(uint8_t val)
{
  if (_val != 0)
  {
    _val_last = _val;
  }
  if (val != 0)
  {
    _val_last = val;
  }
  _val = val;
}

void EspalexaDevice::setPercent(uint8_t perc)
{
  uint16_t val = perc * 255;
  val /= 100;
  if (val > 255) val = 255;
  setValue(val);
}

void EspalexaDevice::setColorXY(float x, float y)
{
  _x = x;
  _y = y;
  _rgb = 0;
  _mode = EspalexaColorMode::xy;
}

void EspalexaDevice::setColor(uint16_t hue, uint8_t sat)
{
  _hue = hue;
  _sat = sat;
  _rgb = 0;
  _mode = EspalexaColorMode::hs;
}

void EspalexaDevice::setColor(uint16_t ct)
{
  _ct = ct;
  _rgb = 0;
  _mode =EspalexaColorMode::ct;
}

void EspalexaDevice::setColor(uint8_t r, uint8_t g, uint8_t b)
{
  float X = r * 0.664511f + g * 0.154324f + b * 0.162028f;
  float Y = r * 0.283881f + g * 0.668433f + b * 0.047685f;
  float Z = r * 0.000088f + g * 0.072310f + b * 0.986039f;
  _x = X / (X + Y + Z);
  _y = Y / (X + Y + Z);
  _rgb = ((r << 16) | (g << 8) | b);
  _mode = EspalexaColorMode::xy;
}

void EspalexaDevice::RGBtoRGBW(byte* rgb) //rgb to rgbw (http://codewelt.com/rgbw)
{
  // https://stackoverflow.com/questions/40312216/converting-rgb-to-rgbw
  float Ri=rgb[0], Gi=rgb[1], Bi=rgb[2];
  //Get the maximum between R, G, and B
  float tM = maxf(Ri, maxf(Gi, Bi));
  
  //If the maximum value is 0, immediately return pure black.
  if(tM == 0) rgb[3] = 0;
  else
  {
    //This section serves to figure out what the color with 100% hue is
    float multiplier = 255.0f / tM;
    float hR = Ri * multiplier;
    float hG = Gi * multiplier;
    float hB = Bi * multiplier;  
    
    //This calculates the Whiteness (not strictly speaking Luminance) of the color
    float M = maxf(hR, maxf(hG, hB));
    float m = minf(hR, minf(hG, hB));
    float Luminance = ((M + m) / 2.0f - 127.5f) * (255.0f/127.5f) / multiplier;
    
    //Calculate and trim the output values
    int Wo = (byte)constrain(Luminance,0.1,255.1);
    int Bo = (byte)constrain(Bi - Luminance,0.1,255.1);
    int Ro = (byte)constrain(Ri - Luminance,0.1,255.1);
    int Go = (byte)constrain(Gi - Luminance,0.1,255.1);
    
    rgb[0]=Ro; rgb[1]=Go; rgb[2]=Bo; rgb[3]=Wo; 
  }
}

void EspalexaDevice::doCallback()
{
  if (_callback != nullptr) {_callback(_val); return;}
  if (_callbackDev != nullptr) {_callbackDev(this); return;}
  if (_callbackCol != nullptr) _callbackCol(_val, getRGB());
}