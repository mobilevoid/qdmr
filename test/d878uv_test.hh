#ifndef D878UVTEST_HH
#define D878UVTEST_HH


#include "libdmrconfigtest.hh"

class D878UVTest : public UnitTestBase
{
  Q_OBJECT

public:
  explicit D878UVTest(QObject *parent = nullptr);

protected:
  void encodeDecode(Config &input, Config &output);

private slots:
  void initTestCase();

  void testBasicConfigEncoding();
  void testBasicConfigDecoding();
  void testChannelFrequency();

  void testAnalogMicGain();
  void testRoaming();
  void testHangTime();
  void testKeyFunctions();

  void testFMAPRSSettings();

  void testAESEncryption();
  void testARC4Encryption();

  void testRegressionDefaultChannel();
  void testRegressionAutoRepeater();
  void testRegressionVFOStep();

  void testEmptyAESKey();  ///< Regression test for #711
  void testChannelDataACK(); ///< Regression test for #813
  void testSettingsDisplayVolumeChangePrompt(); ///< Regression test for #813
  void testRadioLimits(); ///< Regression test for #816
  void testMicGain();
  void testFixedLocation();
  void testHiddenZone(); ///< Regression test for #203
  void testCustomCTCSS(); ///< Regression test fpr #1004

  void testChannelRoundTrip();            ///< Channel flags survive encode/decode.
  void testSettingsOff();                 ///< TOT, RX backlight and APRS period "off".
  void testUpdateKeepsDeviceSettings();   ///< Updating keeps device settings that decode unchanged.
  void testMessageIndex();                ///< Message index is a linked list.
  void testRoamingChannelColorCode();     ///< Roaming channel color code 0 is kept.
  void testDMRAPRSWithoutDestination();   ///< Channels reporting via a GPS system w/o destination.

protected:
  Config _micGainConfig;
  QTextStream _stderr;
};

#endif // D878UVTEST_HH
