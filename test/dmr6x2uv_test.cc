#include "dmr6x2uv_test.hh"
#include "config.hh"
#include "dmr6x2uv_codeplug.hh"
#include "errorstack.hh"
#include "anytone_extension.hh"
#include "anytone_settingsextension.hh"
#include "gpssystem.hh"
#include "scanlist.hh"
#include <QTest>

DMR6X2UVTest::DMR6X2UVTest(QObject *parent)
  : UnitTestBase(parent)
{
  // pass...
}

void
DMR6X2UVTest::encodeDecode(Config &input, Config &output) {
  ErrorStack err;
  D878UVCodeplug codeplug;
  Codeplug::Flags flags; flags.setUpdateCodeplug(false);
  if (! codeplug.encode(&input, flags, err)) {
    QFAIL(QString("Cannot encode codeplug for BTECH DMR-6X2UV: %1")
            .arg(err.format()).toStdString().c_str());
  }
  if (! codeplug.decode(&output, err)) {
    QFAIL(QString("Cannot decode codeplug for BTECH DMR-6X2UV: %1")
            .arg(err.format()).toStdString().c_str());
  }
}


void
DMR6X2UVTest::testBasicConfigEncoding() {
  ErrorStack err;
  Codeplug::Flags flags; flags.setUpdateCodeplug(false);
  DMR6X2UVCodeplug codeplug;
  if (! codeplug.encode(&_basicConfig, flags, err)) {
    QFAIL(QString("Cannot encode codeplug for BTECH DMR-6X2UV: %1")
          .arg(err.format()).toStdString().c_str());
  }
}

void
DMR6X2UVTest::testBasicConfigDecoding() {
  ErrorStack err;
  Codeplug::Flags flags; flags.setUpdateCodeplug(false);
  DMR6X2UVCodeplug codeplug;
  if (! codeplug.encode(&_basicConfig, flags, err)) {
    QFAIL(QString("Cannot encode codeplug for BTECH DMR-6X2UV: %1")
          .arg(err.format()).toStdString().c_str());
  }

  Config config;
  if (! codeplug.decode(&config, err)) {
    QFAIL(QString("Cannot decode codeplug for BETCH DMR-6X2UV: %1")
          .arg(err.format()).toStdString().c_str());
  }
}

void
DMR6X2UVTest::testChannelFrequency() {
  ErrorStack err;
  Codeplug::Flags flags; flags.setUpdateCodeplug(false);
  D868UVCodeplug codeplug;
  codeplug.clear();
  if (! codeplug.encode(&_channelFrequencyConfig, flags, err)) {
    QFAIL(QString("Cannot encode codeplug for BTECH DMR-6X2UV: {}")
          .arg(err.format()).toStdString().c_str());
  }

  Config config;
  if (! codeplug.decode(&config, err)) {
    QFAIL(QString("Cannot decode codeplug for BTECH DMR-6X2UV: {}")
          .arg(err.format()).toStdString().c_str());
  }

  QCOMPARE(config.channelList()->channel(0)->rxFrequency(),
           Frequency::fromHz(123456780ULL));
  QCOMPARE(config.channelList()->channel(0)->txFrequency(),
           Frequency::fromHz(999999990ULL));
}

void
DMR6X2UVTest::testFMAPRSSettings() {
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

  QCOMPARE(config.channelList()->count(), 2);
  QVERIFY(config.channelList()->channel(0)->is<FMChannel>());
  QVERIFY(config.channelList()->channel(1)->is<FMChannel>());
  QVERIFY(! config.channelList()->channel(1)->as<FMChannel>()->aprsRef()->isNull());

  // Add extensions
  auto ch_ext = new AnytoneFMChannelExtension();
  ch_ext->setAPRSPTT(AnytoneChannelExtension::APRSPTT::Start);
  config.channelList()->channel(1)->as<FMChannel>()->setAnytoneChannelExtension(ch_ext);

  // test extension settings
  auto ext = new AnytoneFMAPRSSettingsExtension();
  ext->setPreWaveDelay(Interval::fromMilliseconds(100));
  ext->setTXDelay(Interval::fromMilliseconds(200));
  aprs->setAnytoneExtension(ext);

  // Encode
  DMR6X2UVCodeplug codeplug;
  Codeplug::Flags flags; flags.setUpdateCodeplug(false);
  Config *intermediate = codeplug.preprocess(&config, err);
  if (nullptr == intermediate) {
    QFAIL(QString("Cannot prepare codeplug for BTECH DMR-6X2UV: %1")
          .arg(err.format()).toStdString().c_str());
  }

  if (! codeplug.encode(intermediate, flags, err)) {
    delete intermediate;
    QFAIL(QString("Cannot encode codeplug for BTECH DMR-6X2UV: %1")
          .arg(err.format()).toStdString().c_str());
  }
  delete intermediate;

  // Decode
  Config comp_config;
  if (! codeplug.decode(&comp_config, err)) {
    QFAIL(QString("Cannot decode codeplug for BTECH DMR-6X2UV: %1")
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

  // check extension settings
  QVERIFY(nullptr != comp_aprs->anytoneExtension());
  QCOMPARE(comp_aprs->anytoneExtension()->preWaveDelay().milliseconds(),
           100);
  QCOMPARE(comp_aprs->anytoneExtension()->txDelay().milliseconds(),
           200);

  // Check revert channel
  QCOMPARE(comp_config.channelList()->count(), 2);
  QVERIFY(comp_config.channelList()->channel(0)->is<FMChannel>());
  QCOMPARE(comp_config.channelList()->channel(0)->rxFrequency(),
           config.channelList()->channel(0)->rxFrequency());
  QCOMPARE(comp_config.channelList()->channel(0)->txFrequency(),
           config.channelList()->channel(0)->txFrequency());

  // Check channel extension properties
  QVERIFY(nullptr != comp_config.channelList()->channel(1)->as<FMChannel>()->anytoneChannelExtension());
  auto comp_ch_ext = comp_config.channelList()->channel(1)->as<FMChannel>()->anytoneChannelExtension();
  QCOMPARE(comp_ch_ext->aprsPTT(), ch_ext->aprsPTT());
}


void
DMR6X2UVTest::testAESEncryption() {
  ErrorStack err;

  // Load config from file
  Config config;
  if (! config.readYAML(":/data/aes_encryption.yaml", err)) {
    QFAIL(QString("Cannot open codeplug file:\n%1")
          .arg(err.format(" ")).toStdString().c_str());
  }

  // Check parser
  QVERIFY(nullptr != config.commercialExtension());
  QCOMPARE(config.commercialExtension()->encryptionKeys()->count(), 1);
  QCOMPARE(config.commercialExtension()->encryptionKeys()->key(0)->key(), QByteArray::fromHex("11223344556677889900AABBCCDDEEFF"));
  QVERIFY(nullptr != config.channelList()->channel(0)->as<DMRChannel>()->commercialExtension());
  QCOMPARE(config.channelList()->channel(0)->as<DMRChannel>()->commercialExtension()->encryptionKey(),
           config.commercialExtension()->encryptionKeys()->key(0));

  DMR6X2UVCodeplug codeplug;
  Codeplug::Flags flags; flags.setUpdateCodeplug(false);
  if (! codeplug.encode(&config, flags, err)) {
    QFAIL(QString("Cannot encode codeplug for BETCH DMR6X2UV: %1")
          .arg(err.format()).toStdString().c_str());
  }

  Config decoded;
  if (! codeplug.decode(&decoded, err)) {
    QFAIL(QString("Cannot decode codeplug for BETCH DMR6X2UV: %1")
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
DMR6X2UVTest::testARC4Encryption() {
  ErrorStack err;

  // Load config from file
  Config config;
  if (! config.readYAML(":/data/arc4_encryption.yaml", err)) {
    QFAIL(QString("Cannot open codeplug file:\n%1")
          .arg(err.format(" ")).toStdString().c_str());
  }

  DMR6X2UVCodeplug codeplug;
  Codeplug::Flags flags; flags.setUpdateCodeplug(false);
  if (! codeplug.encode(&config, flags, err)) {
    QFAIL(QString("Cannot encode codeplug for BETCH DMR6X2UV: %1")
          .arg(err.format()).toStdString().c_str());
  }

  Config decoded;
  if (! codeplug.decode(&decoded, err)) {
    QFAIL(QString("Cannot decode codeplug for BETCH DMR6X2UV: %1")
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
DMR6X2UVTest::testMicGain() {
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
DMR6X2UVTest::testHiddenZone() {
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
DMR6X2UVTest::testChannelRoundTrip() {
  ErrorStack err;
  Config config;
  if (! config.readYAML(":/data/anytone_channel_roundtrip.yaml", err))
    QFAIL(QString("Cannot open codeplug file: %1").arg(err.format()).toStdString().c_str());

  DMR6X2UVCodeplug codeplug;
  Codeplug::Flags flags; flags.setUpdateCodeplug(false);
  if (! codeplug.encode(&config, flags, err))
    QFAIL(QString("Cannot encode codeplug: %1").arg(err.format()).toStdString().c_str());

  // Scan list slots: channel 1 has scan list 2 (index 1) as additional list, unused slots are 0xff
  QCOMPARE(codeplug.data(0x00800000)[0x37], (unsigned char)0x01);
  for (unsigned int i=0x38; i<0x3e; i++)
    QCOMPARE(codeplug.data(0x00800000)[i], (unsigned char)0xff);
  for (unsigned int i=0x37; i<0x3e; i++)
    QCOMPARE(codeplug.data(0x00800040)[i], (unsigned char)0xff);

  Config decoded;
  if (! codeplug.decode(&decoded, err))
    QFAIL(QString("Cannot decode codeplug: %1").arg(err.format()).toStdString().c_str());

  DMRChannel *ch1 = decoded.channelList()->channel(0)->as<DMRChannel>();
  QVERIFY(ch1);
  QVERIFY(nullptr != ch1->roaming());
  QCOMPARE(ch1->anytoneChannelExtension()->ranging(), true);
  QCOMPARE(ch1->anytoneChannelExtension()->throughMode(), false);
  QCOMPARE(ch1->anytoneChannelExtension()->additionalScanLists()->count(), 1);
  QCOMPARE(ch1->anytoneChannelExtension()->additionalScanLists()->get(0)->as<ScanList>()->name(),
           QString("Scan 2"));

  DMRChannel *ch2 = decoded.channelList()->channel(1)->as<DMRChannel>();
  QVERIFY(ch2);
  QVERIFY(nullptr == ch2->roaming());
  QCOMPARE(ch2->anytoneChannelExtension()->throughMode(), true);
  QCOMPARE(ch2->extended()->dataConfirm(), false);
  QCOMPARE(ch2->anytoneChannelExtension()->additionalScanLists()->count(), 0);
}


void
DMR6X2UVTest::testSettingsOff() {
  ErrorStack err;
  Config config;
  if (! config.readYAML(":/data/fm_aprs_test.yaml", err))
    QFAIL(QString("Cannot open codeplug file: %1").arg(err.format()).toStdString().c_str());
  if (nullptr == config.settings()->anytoneExtension())
    config.settings()->setAnytoneExtension(new AnytoneSettingsExtension());
  config.settings()->anytoneExtension()->displaySettings()->setBacklightDurationRX(Interval::infinity());
  config.posSystems()->get(0)->as<FMAPRSSystem>()->disablePeriod();

  DMR6X2UVCodeplug codeplug;
  Codeplug::Flags flags; flags.setUpdateCodeplug(false);
  if (! codeplug.encode(&config, flags, err))
    QFAIL(QString("Cannot encode codeplug: %1").arg(err.format()).toStdString().c_str());

  Config decoded;
  if (! codeplug.decode(&decoded, err))
    QFAIL(QString("Cannot decode codeplug: %1").arg(err.format()).toStdString().c_str());
  QVERIFY(decoded.settings()->anytoneExtension()->displaySettings()->backlightDurationRX().isInfinite());
  QVERIFY(! decoded.posSystems()->get(0)->as<FMAPRSSystem>()->period().isFinite());
}


QTEST_GUILESS_MAIN(DMR6X2UVTest)

