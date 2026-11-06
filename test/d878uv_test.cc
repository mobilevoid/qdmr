#include "d878uv_test.hh"
#include "config.hh"
#include "d878uv_codeplug.hh"
#include "d878uv_limits.hh"
#include "errorstack.hh"
#include <QTest>
#include "logger.hh"
#include "anytone_extension.hh"
#include "anytone_settingsextension.hh"
#include "roamingchannel.hh"
#include "roamingzone.hh"
#include "gpssystem.hh"
#include "smsextension.hh"
#include "scanlist.hh"

D878UVTest::D878UVTest(QObject *parent)
  : UnitTestBase(parent), _stderr(stderr)
{
  Logger::get().addHandler(new StreamLogHandler(_stderr, LogMessage::DEBUG));
}

void
D878UVTest::encodeDecode(Config &input, Config &output) {
  ErrorStack err;
  D878UVCodeplug codeplug;
  Codeplug::Flags flags; flags.setUpdateCodeplug(false);
  if (! codeplug.encode(&input, flags, err)) {
    QFAIL(QString("Cannot encode codeplug for AnyTone AT-D878UV: %1")
            .arg(err.format()).toStdString().c_str());
  }
  if (! codeplug.decode(&output, err)) {
    QFAIL(QString("Cannot decode codeplug for AnyTone AT-D878UV: %1")
            .arg(err.format()).toStdString().c_str());
  }
}

void
D878UVTest::initTestCase() {
  UnitTestBase::initTestCase();

  ErrorStack err;
  if (! _micGainConfig.readYAML(":/data/audio_settings_extension.yaml", err)) {
    QFAIL(QString("Cannot open codeplug file: %1")
          .arg(err.format()).toStdString().c_str());
  }
}

void
D878UVTest::testBasicConfigEncoding() {
  ErrorStack err;
  Codeplug::Flags flags; flags.setUpdateCodeplug(false);
  D878UVCodeplug codeplug;
  if (! codeplug.encode(&_basicConfig, flags, err)) {
    QFAIL(QString("Cannot encode codeplug for AnyTone AT-D878UV: %1")
          .arg(err.format()).toStdString().c_str());
  }
}

void
D878UVTest::testBasicConfigDecoding() {
  ErrorStack err;
  Codeplug::Flags flags; flags.setUpdateCodeplug(false);
  D878UVCodeplug codeplug;
  if (! codeplug.encode(&_basicConfig, flags, err)) {
    QFAIL(QString("Cannot encode codeplug for AnyTone AT-D878UV: %1")
          .arg(err.format()).toStdString().c_str());
  }

  Config config;
  if (! codeplug.decode(&config, err)) {
    QFAIL(QString("Cannot decode codeplug for AnyTone AT-D878UV: %1")
          .arg(err.format()).toStdString().c_str());
  }
}

void
D878UVTest::testAnalogMicGain() {
  Config config;
  encodeDecode(_micGainConfig, config);
  QVERIFY(config.settings()->audio()->fmMicGainEnabled());
  QCOMPARE(config.settings()->audio()->fmMicGain(), Level::fromValue(5));
}

void
D878UVTest::testChannelFrequency() {
  ErrorStack err;
  Codeplug::Flags flags; flags.setUpdateCodeplug(false);
  D868UVCodeplug codeplug;
  codeplug.clear();
  if (! codeplug.encode(&_channelFrequencyConfig, flags, err)) {
    QFAIL(QString("Cannot encode codeplug for AnyTone D878UV: {}")
          .arg(err.format()).toStdString().c_str());
  }

  Config config;
  if (! codeplug.decode(&config, err)) {
    QFAIL(QString("Cannot decode codeplug for AnyTone D878UV: {}")
          .arg(err.format()).toStdString().c_str());
  }

  QCOMPARE(config.channelList()->channel(0)->rxFrequency(),
           Frequency::fromHz(123456780ULL));
  QCOMPARE(config.channelList()->channel(0)->txFrequency(),
           Frequency::fromHz(999999990ULL));
}

void
D878UVTest::testRoaming() {
  ErrorStack err;
  Codeplug::Flags flags; flags.setUpdateCodeplug(false);
  D878UVCodeplug codeplug;
  if (! codeplug.encode(&_roamingConfig, flags, err)) {
    QFAIL(QString("Cannot decode codeplug for AnyTone AT-D878UV: {}")
          .arg(err.format()).toStdString().c_str());
  }

  Config config;
  if (! codeplug.decode(&config, err)) {
    QFAIL(QString("Cannot decode codeplug for AnyTone D878UV: {}")
          .arg(err.format()).toStdString().c_str());
  }

  QCOMPARE(config.roamingChannels()->count(), 3);
  QCOMPARE(config.roamingZones()->count(), 2);
  QCOMPARE(config.roamingZones()->count(), 2);

  QCOMPARE(config.roamingZones()->get(0)->as<RoamingZone>()->count(), 2);
  QCOMPARE(config.roamingZones()->get(0)->as<RoamingZone>()->channel(0),
           config.roamingChannels()->get(0)->as<RoamingChannel>());
  QCOMPARE(config.roamingZones()->get(0)->as<RoamingZone>()->channel(1),
           config.roamingChannels()->get(1)->as<RoamingChannel>());

  QCOMPARE(config.roamingZones()->get(1)->as<RoamingZone>()->count(), 2);
  QCOMPARE(config.roamingZones()->get(1)->as<RoamingZone>()->channel(0),
           config.roamingChannels()->get(0)->as<RoamingChannel>());
  QCOMPARE(config.roamingZones()->get(1)->as<RoamingZone>()->channel(1),
           config.roamingChannels()->get(2)->as<RoamingChannel>());
}

void
D878UVTest::testHangTime() {
  ErrorStack err;

  // Load config from file
  Config config;
  if (! config.readYAML(":/data/anytone_call_hangtime.yaml", err)) {
    QFAIL(QString("Cannot open codeplug file: %1")
          .arg(err.format()).toStdString().c_str());
  }

  // Check config
  QVERIFY2(config.settings()->anytoneExtension(), "Expected AnyTone settings extension.");

  QCOMPARE(config.settings()->dmr()->privateCallHangTime().seconds(), 4ULL);
  QCOMPARE(config.settings()->dmr()->groupCallHangTime().seconds(), 5ULL);

  // Encode
  D878UVCodeplug codeplug;
  Codeplug::Flags flags; flags.setUpdateCodeplug(false);
  Config *intermediate = codeplug.preprocess(&config, err);
  if (nullptr == intermediate) {
    QFAIL(QString("Cannot prepare codeplug for AnyTone AT-D878UVE: %1")
          .arg(err.format()).toStdString().c_str());
  }

  if (! codeplug.encode(intermediate, flags, err)) {
    delete intermediate;
    QFAIL(QString("Cannot encode codeplug for AnyTone AT-D878UVE: %1")
          .arg(err.format()).toStdString().c_str());
  }
  delete intermediate;

  // Decode
  Config comp_config;
  if (! codeplug.decode(&comp_config, err)) {
    QFAIL(QString("Cannot decode codeplug for AnyTone AT-D878UVII: %1")
          .arg(err.format()).toStdString().c_str());
  }

  // Check config
  QVERIFY2(comp_config.settings()->anytoneExtension(), "Expected AnyTone settings extension.");

  QCOMPARE(config.settings()->dmr()->privateCallHangTime().seconds(), 4ULL);
  QCOMPARE(config.settings()->dmr()->groupCallHangTime().seconds(), 5ULL);
}

void
D878UVTest::testKeyFunctions() {
  ErrorStack err;

  // Load config from file
  Config config;
  if (! config.readYAML(":/data/anytone_key_function.yaml", err)) {
    QFAIL(QString("Cannot open codeplug file: %1")
          .arg(err.format()).toStdString().c_str());
  }

  // Check config
  QVERIFY2(config.settings()->anytoneExtension(), "Expected AnyTone settings extension.");
  AnytoneKeySettingsExtension *ext = config.settings()->anytoneExtension()->keySettings();

  QCOMPARE(ext->funcKey1Short(), AnytoneKeySettingsExtension::KeyFunction::ToggleMainChannel);
  QCOMPARE(ext->funcKey1Long(), AnytoneKeySettingsExtension::KeyFunction::GPS);
  QCOMPARE(ext->funcKey2Short(), AnytoneKeySettingsExtension::KeyFunction::ToggleVFO);
  QCOMPARE(ext->funcKey2Long(), AnytoneKeySettingsExtension::KeyFunction::ChannelType);
  QCOMPARE(ext->funcKeyAShort(), AnytoneKeySettingsExtension::KeyFunction::Off);
  QCOMPARE(ext->funcKeyALong(), AnytoneKeySettingsExtension::KeyFunction::Encryption);
  QCOMPARE(ext->funcKeyBShort(), AnytoneKeySettingsExtension::KeyFunction::Voltage);
  QCOMPARE(ext->funcKeyBLong(), AnytoneKeySettingsExtension::KeyFunction::Call);
  QCOMPARE(ext->funcKeyBShort(), AnytoneKeySettingsExtension::KeyFunction::Voltage);
  QCOMPARE(ext->funcKeyBLong(), AnytoneKeySettingsExtension::KeyFunction::Call);

  // Encode
  D878UVCodeplug codeplug;
  Codeplug::Flags flags; flags.setUpdateCodeplug(false);
  if (! codeplug.encode(&config, flags, err)) {
    QFAIL(QString("Cannot encode codeplug for AnyTone AT-D868UV: %1")
          .arg(err.format()).toStdString().c_str());
  }

  // Decode
  Config comp_config;
  if (! codeplug.decode(&comp_config, err)) {
    QFAIL(QString("Cannot decode codeplug for AnyTone AT-D878UV: %1")
          .arg(err.format()).toStdString().c_str());
  }

  // Check config
  QVERIFY2(comp_config.settings()->anytoneExtension(), "Expected AnyTone settings extension.");
  ext = comp_config.settings()->anytoneExtension()->keySettings();

  QCOMPARE(ext->funcKey1Short(), AnytoneKeySettingsExtension::KeyFunction::ToggleMainChannel);
  QCOMPARE(ext->funcKey1Long(), AnytoneKeySettingsExtension::KeyFunction::GPS);
  QCOMPARE(ext->funcKey2Short(), AnytoneKeySettingsExtension::KeyFunction::ToggleVFO);
  QCOMPARE(ext->funcKey2Long(), AnytoneKeySettingsExtension::KeyFunction::ChannelType);
  QCOMPARE(ext->funcKeyAShort(), AnytoneKeySettingsExtension::KeyFunction::Off);
  QCOMPARE(ext->funcKeyALong(), AnytoneKeySettingsExtension::KeyFunction::Encryption);
  QCOMPARE(ext->funcKeyBShort(), AnytoneKeySettingsExtension::KeyFunction::Voltage);
  QCOMPARE(ext->funcKeyBLong(), AnytoneKeySettingsExtension::KeyFunction::Call);
  QCOMPARE(ext->funcKeyBShort(), AnytoneKeySettingsExtension::KeyFunction::Voltage);
  QCOMPARE(ext->funcKeyBLong(), AnytoneKeySettingsExtension::KeyFunction::Call);
}


void
D878UVTest::testFMAPRSSettings() {
  ErrorStack err;

  // Load config from file
  Config config;
  if (! config.readYAML(":/data/fm_aprs_test.yaml", err)) {
    QFAIL(QString("Cannot open codeplug file: %1\n")
          .arg(err.format()).toStdString().c_str());
  }

  // Check config
  QCOMPARE(config.posSystems()->count(), 1);
  QVERIFY(config.posSystems()->get(0)->is<FMAPRSSystem>());

  FMAPRSSystem *aprs = config.posSystems()->get(0)->as<FMAPRSSystem>();
  QCOMPARE(aprs->source(), "DM3MAT"); QCOMPARE(aprs->srcSSID(), 7);
  QCOMPARE(aprs->destination(), "APAT81"); QCOMPARE(aprs->destSSID(), 0);
  QCOMPARE(aprs->path(), "WIDE1-1,WIDE2-1");
  QCOMPARE(aprs->period(), Interval::fromMinutes(5));

  // Encode
  D878UVCodeplug codeplug;
  Codeplug::Flags flags; flags.setUpdateCodeplug(false);
  Config *intermediate = codeplug.preprocess(&config, err);
  if (nullptr == intermediate) {
    QFAIL(QString("Cannot prepare codeplug for AnyTone AT-D878UV: %1")
          .arg(err.format()).toStdString().c_str());
  }

  if (! codeplug.encode(intermediate, flags, err)) {
    delete intermediate;
    QFAIL(QString("Cannot encode codeplug for AnyTone AT-D878UV: %1")
          .arg(err.format()).toStdString().c_str());
  }
  delete intermediate;

  // Decode
  Config comp_config;
  if (! codeplug.decode(&comp_config, err)) {
    QFAIL(QString("Cannot decode codeplug for AnyTone AT-D878UV: %1")
          .arg(err.format()).toStdString().c_str());
  }

  // Check config
  QCOMPARE(comp_config.posSystems()->count(), 1);
  QVERIFY(comp_config.posSystems()->get(0)->is<FMAPRSSystem>());

  FMAPRSSystem *comp_aprs = comp_config.posSystems()->get(0)->as<FMAPRSSystem>();
  QCOMPARE(comp_aprs->source(), aprs->source()); QCOMPARE(comp_aprs->srcSSID(), aprs->srcSSID());
  QCOMPARE(comp_aprs->destination(), aprs->destination()); QCOMPARE(comp_aprs->destSSID(), aprs->destSSID());
  QCOMPARE(comp_aprs->path(), aprs->path());
  QCOMPARE(comp_aprs->period(), aprs->period());
}



void
D878UVTest::testAESEncryption() {
  ErrorStack err;

  // Load config from file
  Config config;
  if (! config.readYAML(":/data/aes_encryption.yaml", err)) {
    QFAIL(QString("Cannot open codeplug file:\n%1")
          .arg(err.format(" ")).toStdString().c_str());
  }

  D878UVCodeplug codeplug;
  Codeplug::Flags flags; flags.setUpdateCodeplug(false);
  if (! codeplug.encode(&config, flags, err)) {
    QFAIL(QString("Cannot encode codeplug for AnyTone AT-D878UV: %1")
          .arg(err.format()).toStdString().c_str());
  }

  Config decoded;
  if (! codeplug.decode(&decoded, err)) {
    QFAIL(QString("Cannot decode codeplug for AnyTone AT-D878UV: %1")
          .arg(err.format()).toStdString().c_str());
  }

  // Verify existence of key
  QVERIFY(nullptr != decoded.commercialExtension());
  QCOMPARE(decoded.commercialExtension()->encryptionKeys()->count(), 1);
  QCOMPARE(decoded.commercialExtension()->encryptionKeys()->key(0)->key(), QByteArray::fromHex("11223344556677889900AABBCCDDEEFF"));

  // Verify link to channel
  QVERIFY(nullptr != decoded.channelList()->channel(0)->as<DMRChannel>()->commercialExtension());
  QCOMPARE(decoded.channelList()->channel(0)->as<DMRChannel>()->commercialExtension()->encryptionKey(),
           decoded.commercialExtension()->encryptionKeys()->key(0));
}


void
D878UVTest::testARC4Encryption() {
  ErrorStack err;

  // Load config from file
  Config config;
  if (! config.readYAML(":/data/arc4_encryption.yaml", err)) {
    QFAIL(QString("Cannot open codeplug file:\n%1")
          .arg(err.format(" ")).toStdString().c_str());
  }

  D878UVCodeplug codeplug;
  Codeplug::Flags flags; flags.setUpdateCodeplug(false);
  if (! codeplug.encode(&config, flags, err)) {
    QFAIL(QString("Cannot encode codeplug for AnyTone AT-D878UV: %1")
          .arg(err.format()).toStdString().c_str());
  }

  Config decoded;
  if (! codeplug.decode(&decoded, err)) {
    QFAIL(QString("Cannot decode codeplug for AnyTone AT-D878UV: %1")
          .arg(err.format()).toStdString().c_str());
  }

  // Verify existence of key
  QVERIFY(nullptr != decoded.commercialExtension());
  QCOMPARE(decoded.commercialExtension()->encryptionKeys()->count(), 1);
  QCOMPARE(decoded.commercialExtension()->encryptionKeys()->key(0)->key(), QByteArray::fromHex("1122334455"));

  // Verify link to channel
  QVERIFY(nullptr != decoded.channelList()->channel(0)->as<DMRChannel>()->commercialExtension());
  QCOMPARE(decoded.channelList()->channel(0)->as<DMRChannel>()->commercialExtension()->encryptionKey(),
           decoded.commercialExtension()->encryptionKeys()->key(0));
}


void
D878UVTest::testRegressionDefaultChannel() {
  ErrorStack err;

  // Load config from file
  Config config;
  if (! config.readYAML(":/data/config_test.yaml", err)) {
    QFAIL(QString("Cannot open codeplug file: %1")
          .arg(err.format()).toStdString().c_str());
  }
  config.settings()->setAnytoneExtension(new AnytoneSettingsExtension());
  config.settings()->boot()->zoneA()->set(config.zones()->zone(0));
  config.settings()->boot()->channelA()->set(config.zones()->zone(0)->A()->get(0));
  config.settings()->boot()->zoneB()->set(config.zones()->zone(1));
  config.settings()->boot()->channelB()->set(config.zones()->zone(1)->A()->get(0));
  config.settings()->boot()->enableDefaultChannel(true);

  D878UVCodeplug codeplug;
  Codeplug::Flags flags; flags.setUpdateCodeplug(false);
  if (! codeplug.encode(&config, flags, err)) {
    QFAIL(QString("Cannot encode codeplug for AnyTone AT-D878UV: %1")
          .arg(err.format()).toStdString().c_str());
  }

  Config decoded;
  if (! codeplug.decode(&decoded, err)) {
      QFAIL(QString("Cannot decode codeplug for AnyTone AT-D878UV: %1")
            .arg(err.format()).toStdString().c_str());
  }

  QVERIFY(decoded.settings()->boot()->defaultChannelEnabled());
  QCOMPARE(decoded.settings()->boot()->zoneA()->as<Zone>(),
           decoded.zones()->zone(0));
  QCOMPARE(decoded.settings()->boot()->channelA()->as<Channel>()->name(),
           decoded.zones()->zone(0)->A()->get(0)->name());
  QCOMPARE(decoded.settings()->boot()->zoneB()->as<Zone>(),
           decoded.zones()->zone(1));
  QCOMPARE(decoded.settings()->boot()->channelB()->as<Channel>()->name(),
           decoded.zones()->zone(1)->A()->get(0)->name());
}



void
D878UVTest::testRegressionAutoRepeater() {
  ErrorStack err;

  // Load config from file
  Config config;
  if (! config.readYAML(":/data/config_test.yaml", err)) {
    QFAIL(QString("Cannot open codeplug file: %1")
          .arg(err.format()).toStdString().c_str());
  }
  config.settings()->setAnytoneExtension(new AnytoneSettingsExtension());

  D878UVCodeplug codeplug;
  Codeplug::Flags flags; flags.setUpdateCodeplug(false);
  if (! codeplug.encode(&config, flags, err)) {
    QFAIL(QString("Cannot encode codeplug for AnyTone AT-D878UV: %1")
          .arg(err.format()).toStdString().c_str());
  }

  Config decoded;
  if (! codeplug.decode(&decoded, err)) {
      QFAIL(QString("Cannot decode codeplug for AnyTone AT-D878UV: %1")
            .arg(err.format()).toStdString().c_str());
  }

  QVERIFY(decoded.settings()->anytoneExtension());
  QCOMPARE(decoded.settings()->anytoneExtension()->autoRepeaterSettings()->vhf2Min().inMHz(), 136.0);
  QCOMPARE(decoded.settings()->anytoneExtension()->autoRepeaterSettings()->vhf2Max().inMHz(), 174.0);
  QCOMPARE(decoded.settings()->anytoneExtension()->autoRepeaterSettings()->uhf2Min().inMHz(), 400.0);
  QCOMPARE(decoded.settings()->anytoneExtension()->autoRepeaterSettings()->uhf2Max().inMHz(), 480.0);
}


void
D878UVTest::testRegressionVFOStep() {
  ErrorStack err;

  // Load config from file
  Config config;
  if (! config.readYAML(":/data/config_test.yaml", err)) {
    QFAIL(QString("Cannot open codeplug file: %1")
          .arg(err.format()).toStdString().c_str());
  }
  config.settings()->setAnytoneExtension(new AnytoneSettingsExtension());
  config.settings()->anytoneExtension()->setVFOStep(Frequency::fromkHz(8.33));

  D878UVCodeplug codeplug;
  Codeplug::Flags flags; flags.setUpdateCodeplug(false);
  if (! codeplug.encode(&config, flags, err)) {
    QFAIL(QString("Cannot encode codeplug for AnyTone AT-D878UV: %1")
          .arg(err.format()).toStdString().c_str());
  }

  Config decoded;
  if (! codeplug.decode(&decoded, err)) {
      QFAIL(QString("Cannot decode codeplug for AnyTone AT-D878UV: %1")
            .arg(err.format()).toStdString().c_str());
  }

  QVERIFY(decoded.settings()->anytoneExtension());
  QCOMPARE(decoded.settings()->anytoneExtension()->vfoStep().inkHz(), 8.33);
}


void
D878UVTest::testEmptyAESKey() {
  ErrorStack err;

  // Load config from file
  Config config;
  if (! config.readYAML(":/data/config_test.yaml", err)) {
    QFAIL(QString("Cannot open codeplug file: %1")
          .arg(err.format()).toStdString().c_str());
  }
  // Empty encryption key
  config.commercialExtension()->encryptionKeys()->add(new AESEncryptionKey());

  D878UVCodeplug codeplug;
  Codeplug::Flags flags; flags.setUpdateCodeplug(false);
  if (! codeplug.encode(&config, flags, err)) {
    QFAIL(QString("Cannot encode codeplug for AnyTone AT-D878UV: %1")
          .arg(err.format()).toStdString().c_str());
  }

  Config decoded;
  if (! codeplug.decode(&decoded, err)) {
      QFAIL(QString("Cannot decode codeplug for AnyTone AT-D878UV: %1")
            .arg(err.format()).toStdString().c_str());
  }

  QCOMPARE(decoded.commercialExtension()->encryptionKeys()->count(), 0);
}


void
D878UVTest::testChannelDataACK() {
  ErrorStack err;
  Config config;

  if (! config.readYAML(":/data/anytone_channel_data_ack.yaml", err)) {
    QFAIL(QString("Cannot open codeplug file: %1")
            .arg(err.format()).toStdString().c_str());
  }

  QCOMPARE(config.channelList()->count(), 2);
  QVERIFY(config.channelList()->channel(0)->is<DMRChannel>());
  QCOMPARE(config.channelList()->channel(0)->as<DMRChannel>()
             ->extended()->dataConfirm(), false);
  QVERIFY(config.channelList()->channel(1)->is<DMRChannel>());
  QCOMPARE(config.channelList()->channel(1)->as<DMRChannel>()
             ->extended()->dataConfirm(), true);

  Codeplug::Flags flags; flags.setUpdateCodeplug(false);
  D878UVCodeplug codeplug;
  codeplug.clear();
  if (! codeplug.encode(&config, flags, err)) {
    QFAIL(QString("Cannot encode codeplug for AnyTone D878UV: %1")
            .arg(err.format()).toStdString().c_str());
  }

  Config testConfig;
  if (! codeplug.decode(&testConfig, err)) {
    QFAIL(QString("Cannot decode codeplug for AnyTone D878UV: %1")
            .arg(err.format()).toStdString().c_str());
  }

  QCOMPARE(testConfig.channelList()->channel(0)->as<DMRChannel>()
             ->extended()->dataConfirm(), false);
  QCOMPARE(testConfig.channelList()->channel(1)->as<DMRChannel>()
             ->extended()->dataConfirm(), true);
}


void
D878UVTest::testSettingsDisplayVolumeChangePrompt() {
  ErrorStack err;
  Config config;

  if (! config.readYAML(":/data/anytone_settings_display.yaml", err)) {
    QFAIL(QString("Cannot open codeplug file: %1")
            .arg(err.format()).toStdString().c_str());
  }

  QCOMPARE(config.settings()->anytoneExtension()->displaySettings()->volumeChangePromptEnabled(), false);

  Codeplug::Flags flags; flags.setUpdateCodeplug(false);
  D878UVCodeplug codeplug;
  codeplug.clear();
  if (! codeplug.encode(&config, flags, err)) {
    QFAIL(QString("Cannot encode codeplug for AnyTone D878UV: %1")
            .arg(err.format()).toStdString().c_str());
  }

  Config testConfig;
  if (! codeplug.decode(&testConfig, err)) {
    QFAIL(QString("Cannot decode codeplug for AnyTone D878UV: %1")
            .arg(err.format()).toStdString().c_str());
  }

  QCOMPARE(testConfig.settings()->anytoneExtension()->displaySettings()->volumeChangePromptEnabled(), false);
}


void
D878UVTest::testRadioLimits() {
  D878UVLimits limits({{Frequency::fromMHz(137),Frequency::fromMHz(150)},
                       {Frequency::fromMHz(400),Frequency::fromMHz(450)}},
                      {{Frequency::fromMHz(137),Frequency::fromMHz(150)},
                       {Frequency::fromMHz(400),Frequency::fromMHz(450)}}, "V101");
  RadioLimitContext ctx;
  if (! limits.verifyConfig(&_basicConfig, ctx)) {
    QString issues;
    for (int i=0; i<ctx.count(); i++) {
      if (! issues.isEmpty())
        issues.append(", ");
      issues.append(ctx.message(i).format());
    }
    QFAIL(issues.toLatin1().constData());
  }
}


void
D878UVTest::testMicGain() {
  ErrorStack err;
  Config copy, config; config.copy(_basicConfig);
  QList<QPair<unsigned int,unsigned int>> pairs = {{1,1}, {2,1}, {3,3}, {4,3}, {5,5}, {6,5}, {7,7}, {8,7}, {9,9}, {10,9}};
  for (auto pair: pairs) {
    config.settings()->audio()->setMicGain(Level::fromValue(pair.first));
    encodeDecode(config, copy);
    QCOMPARE(copy.settings()->audio()->micGain(), Level::fromValue(pair.second));
  }
}


void
D878UVTest::testFixedLocation() {
  ErrorStack err;
  Config decoded, config; config.copy(_basicConfig);

  config.settings()->gnss()->setFixedPositionLocator("JO62kk45");
  config.settings()->gnss()->enableFixedPosition(true);
  encodeDecode(config, decoded);

  QVERIFY(decoded.settings()->gnss()->fixedPositionEnabled());
  QCOMPARE(decoded.settings()->gnss()->fixedPositionLocator(), QString("JO62kk45"));
}


void
D878UVTest::testHiddenZone() {
  ErrorStack err;
  Config decoded, config;
  if (! config.readYAML(":/data/anytone_hidden_zones.yaml", err)) {
    QFAIL(QString("Cannot open codeplug file: %1")
            .arg(err.format()).toStdString().c_str());
  }

  encodeDecode(config, decoded);

  QCOMPARE(decoded.zones()->count(), 1);
  QVERIFY(decoded.zones()->zone(0)->anytoneExtension());
  QVERIFY(decoded.zones()->zone(0)->anytoneExtension()->hidden());
}

void
D878UVTest::testCustomCTCSS() {
  ErrorStack err;
  Config decoded, config;
  if (! config.readYAML(":/data/ctcss_copy_test.yaml", err)) {
    QFAIL(QString("Cannot open codeplug file: %1")
            .arg(err.format()).toStdString().c_str());
  }

  config.channelList()->channel(0)->as<FMChannel>()->setRXTone({206.6});
  config.channelList()->channel(0)->as<FMChannel>()->setTXTone({206.6});

  encodeDecode(config, decoded);

  QCOMPARE(decoded.channelList()->count(), 1);
  QVERIFY(decoded.channelList()->channel(0)->is<FMChannel>());
  QCOMPARE(decoded.channelList()->channel(0)->as<FMChannel>()->rxTone(), SelectiveCall(206.6));
  QCOMPARE(decoded.channelList()->channel(0)->as<FMChannel>()->txTone(), SelectiveCall(206.6));
}


void
D878UVTest::testChannelRoundTrip() {
  ErrorStack err;
  Config config;
  if (! config.readYAML(":/data/anytone_channel_roundtrip.yaml", err))
    QFAIL(QString("Cannot open codeplug file: %1").arg(err.format()).toStdString().c_str());

  Config decoded;
  encodeDecode(config, decoded);
  QCOMPARE(decoded.channelList()->count(), 3);

  // Channel 1: roaming, ranging, talker alias, auto scan, frequency correction
  DMRChannel *ch1 = decoded.channelList()->channel(0)->as<DMRChannel>();
  QVERIFY(ch1);
  QVERIFY(nullptr != ch1->roaming());
  QVERIFY(ch1->anytoneChannelExtension());
  QCOMPARE(ch1->anytoneChannelExtension()->throughMode(), false);
  QCOMPARE(ch1->anytoneChannelExtension()->ranging(), true);
  QCOMPARE(ch1->anytoneChannelExtension()->sendTalkerAlias(), true);
  QCOMPARE(ch1->anytoneChannelExtension()->autoScan(), true);
  QCOMPARE(ch1->anytoneChannelExtension()->frequencyCorrection(), 20);

  // Channel 2: excluded from roaming, through mode, SMS disabled, no data ACK
  DMRChannel *ch2 = decoded.channelList()->channel(1)->as<DMRChannel>();
  QVERIFY(ch2);
  QVERIFY(nullptr == ch2->roaming());
  QCOMPARE(ch2->anytoneChannelExtension()->throughMode(), true);
  QCOMPARE(ch2->anytoneChannelExtension()->sendTalkerAlias(), false);
  QCOMPARE(ch2->extended()->sms(), false);
  QCOMPARE(ch2->extended()->dataConfirm(), false);

  // Channel 3: FM signaling settings
  FMChannel *ch3 = decoded.channelList()->channel(2)->as<FMChannel>();
  QVERIFY(ch3);
  AnytoneFMChannelExtension *ext = ch3->anytoneChannelExtension();
  QVERIFY(ext);
  QCOMPARE(ext->ctcssPhaseReversal(), true);
  QCOMPARE(ext->pttID(), AnytoneFMChannelExtension::PTTId::Both);
  QCOMPARE(ext->optionalSignaling(), AnytoneFMChannelExtension::OptionalSignaling::DTMF);
  QCOMPARE(ext->dtmfID(), 2U);
  QCOMPARE(ext->twoToneID(), 1U);
  QCOMPARE(ext->fiveToneID(), 3U);
  QCOMPARE(ext->twoToneDecode(), 4U);
  QCOMPARE(ext->fiveToneBOT(), 1U);
  QCOMPARE(ext->fiveToneEOT(), 2U);
  QCOMPARE(ext->fmAPRSMute(), true);
  QCOMPARE(ext->sendTalkerAlias(), true);
}


void
D878UVTest::testSettingsOff() {
  ErrorStack err;
  Config config;
  if (! config.readYAML(":/data/fm_aprs_test.yaml", err))
    QFAIL(QString("Cannot open codeplug file: %1").arg(err.format()).toStdString().c_str());
  config.settings()->disableTOT();
  if (nullptr == config.settings()->anytoneExtension())
    config.settings()->setAnytoneExtension(new AnytoneSettingsExtension());
  config.settings()->anytoneExtension()->displaySettings()->setBacklightDurationRX(Interval::infinity());
  config.posSystems()->get(0)->as<FMAPRSSystem>()->disablePeriod();

  D878UVCodeplug codeplug;
  Codeplug::Flags flags; flags.setUpdateCodeplug(false);
  if (! codeplug.encode(&config, flags, err))
    QFAIL(QString("Cannot encode codeplug: %1").arg(err.format()).toStdString().c_str());

  // 0 means "off" on the device
  QCOMPARE(*codeplug.data(0x02500004), (unsigned char)0x00); // TOT
  QCOMPARE(*codeplug.data(0x025000e6), (unsigned char)0x00); // RX backlight duration
  QCOMPARE(*codeplug.data(0x0250100b), (unsigned char)0x00); // APRS auto TX interval

  Config decoded;
  if (! codeplug.decode(&decoded, err))
    QFAIL(QString("Cannot decode codeplug: %1").arg(err.format()).toStdString().c_str());
  QVERIFY(decoded.settings()->totDisabled());
  QVERIFY(decoded.settings()->anytoneExtension()->displaySettings()->backlightDurationRX().isInfinite());
  QVERIFY(! decoded.posSystems()->get(0)->as<FMAPRSSystem>()->period().isFinite());
}


void
D878UVTest::testUpdateKeepsDeviceSettings() {
  ErrorStack err;
  D878UVCodeplug codeplug;
  Codeplug::Flags flags; flags.setUpdateCodeplug(false);
  if (! codeplug.encode(&_basicConfig, flags, err))
    QFAIL(QString("Cannot encode codeplug: %1").arg(err.format()).toStdString().c_str());

  // Put device values into the codeplug that the common level scale cannot represent exactly.
  *codeplug.data(0x02500009) = 0x03;  // squelch A
  *codeplug.data(0x0250000a) = 0x03;  // squelch B
  *codeplug.data(0x0250003b) = 0x08;  // max speaker volume [0,8]
  *codeplug.data(0x02500000) = 0x00;  // key tone off
  *codeplug.data(0x025000bb) = 0x0d;  // key tone level [0,15]
  *codeplug.data(0x02501001) = 0x14;  // unknown APRS setting byte
  *codeplug.data(0x0250100a) = 0x2d;  // APRS manual TX interval (not part of the config)
  QByteArray settings((const char *)codeplug.data(0x02500000), 0xf0);
  QByteArray aprs((const char *)codeplug.data(0x02501000), 0x100);

  Config decoded;
  if (! codeplug.decode(&decoded, err))
    QFAIL(QString("Cannot decode codeplug: %1").arg(err.format()).toStdString().c_str());
  QCOMPARE(decoded.settings()->tone()->keyToneEnabled(), false);

  // Encode the decoded config again into the same codeplug, updating it (as during a write).
  Codeplug::Flags update; update.setUpdateCodeplug(true);
  if (! codeplug.encode(&decoded, update, err))
    QFAIL(QString("Cannot encode codeplug: %1").arg(err.format()).toStdString().c_str());

  for (int i=0; i<settings.size(); i++)
    QVERIFY2(settings.at(i) == (char)codeplug.data(0x02500000)[i],
             QString("General settings byte %1 changed").arg(i, 2, 16).toStdString().c_str());
  for (int i=0; i<0x100; i++)
    QVERIFY2(aprs.at(i) == (char)codeplug.data(0x02501000)[i],
             QString("APRS settings byte %1 changed").arg(i, 2, 16).toStdString().c_str());
}


void
D878UVTest::testMessageIndex() {
  ErrorStack err;
  Config config;
  if (! config.readYAML(":/data/config_test.yaml", err))
    QFAIL(QString("Cannot open codeplug file: %1").arg(err.format()).toStdString().c_str());
  for (int i=0; i<3; i++) {
    SMSTemplate *sms = new SMSTemplate();
    sms->setName(QString("SMS %1").arg(i+1));
    sms->setMessage(QString("Message %1").arg(i+1));
    config.smsExtension()->smsTemplates()->add(sms);
  }

  D878UVCodeplug codeplug;
  Codeplug::Flags flags; flags.setUpdateCodeplug(false);
  if (! codeplug.encode(&config, flags, err))
    QFAIL(QString("Cannot encode codeplug: %1").arg(err.format()).toStdString().c_str());

  // Each entry holds the index of the next message (byte 2) and its own index (byte 3).
  QCOMPARE(codeplug.data(0x01640000)[2], (unsigned char)0x01);
  QCOMPARE(codeplug.data(0x01640000)[3], (unsigned char)0x00);
  QCOMPARE(codeplug.data(0x01640010)[2], (unsigned char)0x02);
  QCOMPARE(codeplug.data(0x01640010)[3], (unsigned char)0x01);
  QCOMPARE(codeplug.data(0x01640020)[2], (unsigned char)0xff);
  QCOMPARE(codeplug.data(0x01640020)[3], (unsigned char)0x02);

  Config decoded;
  if (! codeplug.decode(&decoded, err))
    QFAIL(QString("Cannot decode codeplug: %1").arg(err.format()).toStdString().c_str());
  QCOMPARE(decoded.smsExtension()->smsTemplates()->count(), 3);
  QCOMPARE(decoded.smsExtension()->smsTemplates()->get(2)->as<SMSTemplate>()->message(),
           QString("Message 3"));
}


void
D878UVTest::testRoamingChannelColorCode() {
  Config config; config.copy(_roamingConfig);
  RoamingChannel *rch = config.roamingChannels()->get(0)->as<RoamingChannel>();
  rch->overrideColorCode(true);
  rch->setColorCode(0);

  Config decoded;
  encodeDecode(config, decoded);
  RoamingChannel *drch = decoded.roamingChannels()->get(0)->as<RoamingChannel>();
  QVERIFY(drch->colorCodeOverridden());
  QCOMPARE(drch->colorCode(), 0U);
}


void
D878UVTest::testDMRAPRSWithoutDestination() {
  ErrorStack err;
  Config config;
  if (! config.readYAML(":/data/config_test.yaml", err))
    QFAIL(QString("Cannot open codeplug file: %1").arg(err.format()).toStdString().c_str());
  // Remove all existing positioning systems, add one without destination contact
  while (config.posSystems()->count())
    config.posSystems()->del(config.posSystems()->get(0));
  DMRAPRSSystem *sys = new DMRAPRSSystem("GPS", nullptr, nullptr, Interval::fromMinutes(5));
  config.posSystems()->add(sys);
  DMRChannel *ch = nullptr;
  for (int i=0; i<config.channelList()->count(); i++) {
    if ((ch = config.channelList()->channel(i)->as<DMRChannel>()))
      break;
  }
  QVERIFY(ch);
  ch->setAPRS(sys);

  Config decoded;
  encodeDecode(config, decoded);
  QCOMPARE(decoded.posSystems()->count(), 1);
  QVERIFY(decoded.posSystems()->get(0)->is<DMRAPRSSystem>());
  QVERIFY(! decoded.posSystems()->get(0)->as<DMRAPRSSystem>()->hasContact());
  int idx = config.channelList()->indexOf(ch);
  QCOMPARE(decoded.channelList()->channel(idx)->name(), ch->name());
  QVERIFY(decoded.channelList()->channel(idx)->as<DMRChannel>()->aprs());
}


QTEST_GUILESS_MAIN(D878UVTest)

