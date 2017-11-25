#include <SPI.h>

#include <NeoPixelBus.h>
#include <NeoPixelAnimator.h>
#include <NeoPixelBrightnessBus.h>

const uint16_t zeppelinPixelCount = 150;
const uint8_t zeppelinPin = 2;

const uint8_t cloud1Pin = 2;
bool cloud1Struck = false;

const uint8_t cloud2Pin = 7;
bool cloud2Struck = false;

const uint8_t cloud3Pin = 12;
bool cloud3Struck = false;

const uint16_t maxCloudPixelCount = 33;

const uint16_t flashChannce = 1;
const uint16_t flashDuration = 70;
const uint16_t minFlashBrightness = 2;
const uint16_t maxFlashBrightness = 255;

const uint8_t AnimationChannels = 1;

RgbColor white(255, 255, 255);

NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> zeppelinStrip(zeppelinPixelCount, zeppelinPin);
NeoPixelBrightnessBus<NeoGrbFeature, Neo800KbpsMethod> cloud1Strip(maxCloudPixelCount, cloud1Pin);
NeoPixelBrightnessBus<NeoGrbFeature, Neo800KbpsMethod> cloud2Strip(maxCloudPixelCount, cloud2Pin);
NeoPixelBrightnessBus<NeoGrbFeature, Neo800KbpsMethod> cloud3Strip(maxCloudPixelCount, cloud3Pin);

NeoPixelAnimator zeppelinAnimation(AnimationChannels);

NeoPixelAnimator cloudAnimation(1);

// what is stored for state is specific to the need, in this case, the colors.
// basically what ever you need inside the animation update function
struct zeppelinAnimationState
{
  RgbColor StartingColor;
  RgbColor EndingColor;
};

// one entry per pixel to match the animation timing manager
zeppelinAnimationState animationState[AnimationChannels];

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

void FadeInFadeOutRinseRepeat(float luminance)
{
  // Fade upto a random color
  // we use HslColor object as it allows us to easily pick a hue
  // with the same saturation and luminance so the colors picked
  // will have similiar overall brightness
  RgbColor target = HslColor(random(360) / 360.0f, 1.0f, luminance);
  uint16_t time = random(800, 2000);

  animationState[0].StartingColor = zeppelinStrip.GetPixelColor(0);
  animationState[0].EndingColor = target;

  zeppelinAnimation.StartAnimation(0, time, BlendAnimUpdate);
}

void flashAnimation(const AnimationParam& param) {
  uint16_t dice;

  if (!cloud1Struck) {
    dice = random(0, 200);

    if (dice < flashChannce) {
      cloud1Struck = true;
    }
  }

  if (!cloud2Struck) {
    dice = random(0, 200);

    if (dice < flashChannce) {
      cloud2Struck = true;
    }
  }

  if (!cloud3Struck) {
    dice = random(0, 200);

    if (dice < flashChannce) {
      cloud3Struck = true;
    }
  }

  if (cloud1Struck) {
    cloud1Strip.SetBrightness((maxFlashBrightness * param.progress) + minFlashBrightness);
  }

  if (cloud2Struck) {
    cloud2Strip.SetBrightness((maxFlashBrightness * param.progress) + minFlashBrightness);
  }

  if (cloud3Struck) {
    cloud3Strip.SetBrightness((maxFlashBrightness * param.progress) + minFlashBrightness);
  }

  if (param.state == AnimationState_Completed)
  {
    cloud1Struck = false;
    cloud2Struck = false;
    cloud3Struck = false;
    
    cloud1Strip.SetBrightness(minFlashBrightness);
    cloud2Strip.SetBrightness(minFlashBrightness);
    cloud3Strip.SetBrightness(minFlashBrightness);
    
    cloudAnimation.RestartAnimation(param.index);
  }
}

void setupClouds() {
  cloud1Strip.Begin();
  cloud2Strip.Begin();
  cloud3Strip.Begin();

  for (uint16_t pixel = 0; pixel < maxCloudPixelCount; pixel++)
  {
    cloud1Strip.SetPixelColor(pixel, white);
    cloud2Strip.SetPixelColor(pixel, white);
    cloud3Strip.SetPixelColor(pixel, white);
  }

  cloud1Strip.SetBrightness(minFlashBrightness);
  cloud2Strip.SetBrightness(minFlashBrightness);
  cloud3Strip.SetBrightness(minFlashBrightness);

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
    FadeInFadeOutRinseRepeat(0.5f); // 0.0 = black, 0.25 is normal, 0.5 is bright
  }
} 
