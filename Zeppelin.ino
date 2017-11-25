#include <SPI.h>

#include <NeoPixelBus.h>
#include <NeoPixelAnimator.h>
#include <NeoPixelBrightnessBus.h>

struct ZeppelinAnimationState
{
  RgbColor StartingColor;
  RgbColor EndingColor;
};


struct CloudState {
  uint8_t pin;
  uint8_t pixelCount;
  
  bool struck;
  uint8_t flashOffset;
  uint8_t flashCount;
};


const uint16_t zeppelinPixelCount = 150;
const uint8_t zeppelinPin = 13;

const uint8_t cloudCount = 3;

//TODO: make more beautifull
CloudState clouds[cloudCount] = {
  {
    2,      //pin
    17,     //count
    
    false,
    0,
    0    
  },

  {
    7,      //pin
    33,     //count
    
    false,
    0,
    0    
  },

  {
    12,      //pin
    255,     //count
    
    false,
    0,
    0    
  },

};

const uint8_t maxCloudPixelCount = 33;

const uint8_t flashChannce = 1;
const uint8_t flashDuration = 70;
const uint8_t minFlashPixels = 1;
const uint8_t maxFlashPixels = 10;

HslColor normalCloudColor(1.0f, 0, 0.004);

const uint8_t AnimationChannels = 1;


NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> zeppelinStrip(zeppelinPixelCount, zeppelinPin);

//TODO: make clouds strips as arrays
NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> cloud1Strip(clouds[0].pixelCount, clouds[0].pin);
NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> cloud2Strip(clouds[1].pixelCount, clouds[1].pin);
NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> cloud3Strip(clouds[2].pixelCount, clouds[2].pin);


NeoPixelAnimator zeppelinAnimation(AnimationChannels);

NeoPixelAnimator cloudAnimation(1);

// one entry per pixel to match the animation timing manager
ZeppelinAnimationState animationState[AnimationChannels];

void SetRandomSeed()
{
  uint32_t seed;

  // random works best with a seed that can use 31 bits
  // analogRead on a unconnected pin tends toward less than four bits
  seed = analogRead(0);
  delay(1);

  for (int shifts = 3; shifts < 31; shifts += 3)
  {
    seed ^= analogRead(0) << shifts;
    delay(1);
  }

  randomSeed(seed);
}

// simple blend function
void BlendAnimUpdate(const AnimationParam& param)
{
  // this gets called for each animation on every time step
  // progress will start at 0.0 and end at 1.0
  // we use the blend function on the RgbColor to mix
  // color based on the progress given to us in the animation
  RgbColor updatedColor = RgbColor::LinearBlend(
    animationState[param.index].StartingColor,
    animationState[param.index].EndingColor,
    param.progress);

  // apply the color to the strip
  for (uint16_t pixel = 0; pixel < zeppelinPixelCount; pixel++)
  {
    zeppelinStrip.SetPixelColor(pixel, updatedColor);
  }
}


void resetClouds() {
  for (uint8_t f = 0; f < cloudCount; f++) {
    clouds[f].struck = false;
  }

  for (uint8_t f = 0; f < maxCloudPixelCount; f++) {
    cloud1Strip.SetPixelColor(f, normalCloudColor);
    cloud2Strip.SetPixelColor(f, normalCloudColor);
    cloud3Strip.SetPixelColor(f, normalCloudColor);
  }
}

void flashAnimation(const AnimationParam& param) {
  uint16_t dice;

  for (uint8_t f = 0; f < cloudCount; f++) {
    // roll dice for a flash
    if (!clouds[f].struck) {
      dice = random(0, 100);
  
      if (dice < flashChannce) {
        clouds[f].struck = true;
        clouds[f].flashOffset = random(0, clouds[f].pixelCount - minFlashPixels);
        clouds[f].flashCount = min(random(clouds[f].flashOffset, clouds[f].pixelCount), maxFlashPixels);
      }
    }
  }
  
  HslColor flashCloudColor(1.0f, 0, (0.5 * param.progress) + 0.004);
  
  if (clouds[0].struck) {
    for (uint8_t f = clouds[0].flashOffset; f < clouds[0].flashCount; f++) {
      cloud1Strip.SetPixelColor(f, flashCloudColor);
    }
  }

  if (clouds[1].struck) {
    for (uint8_t f = clouds[1].flashOffset; f < clouds[1].flashCount; f++) {
      cloud2Strip.SetPixelColor(f, flashCloudColor);
    }
  }

  if (clouds[2].struck) {
    for (uint8_t f = clouds[2].flashOffset; f < clouds[2].flashCount; f++) {
      cloud3Strip.SetPixelColor(f, flashCloudColor);
    }
  }

  if (param.state == AnimationState_Completed)
  {
    resetClouds();
    cloudAnimation.RestartAnimation(param.index);
  }
}

void setupClouds() {
  cloud1Strip.Begin();
  cloud2Strip.Begin();
  cloud3Strip.Begin();

  resetClouds();

  cloud1Strip.Show();
  cloud2Strip.Show();
  cloud3Strip.Show();

  cloudAnimation.StartAnimation(0, flashDuration, flashAnimation);
}

void setup()
{
  zeppelinStrip.Begin();
  zeppelinStrip.Show();


  SetRandomSeed();
  setupClouds();
}

void loop()
{
  cloudAnimation.UpdateAnimations();
  cloud1Strip.Show();
  cloud2Strip.Show();
  cloud3Strip.Show();

  if (zeppelinAnimation.IsAnimating())
  {
    // the normal loop just needs these two to run the active animations
    zeppelinAnimation.UpdateAnimations();
    zeppelinStrip.Show();
  }
  else
  {
    // no animation runnning, start some
    RgbColor target = HslColor(random(360) / 360.0f, 1.0f, 0.5f);
    uint16_t time = random(800, 2000);
  
    animationState[0].StartingColor = zeppelinStrip.GetPixelColor(0);
    animationState[0].EndingColor = target;
  
    zeppelinAnimation.StartAnimation(0, time, BlendAnimUpdate);
  }
} 
