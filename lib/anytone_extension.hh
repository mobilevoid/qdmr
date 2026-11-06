#ifndef ANYTONEEXTENSION_HH
#define ANYTONEEXTENSION_HH

#include "configobject.hh"
#include "configreference.hh"
#include "frequency.hh"
#include "interval.hh"

#include <QTimeZone>



/** Implements the config representation of an FM APRS frequency.
 * @ingroup anytone */
class AnytoneAPRSFrequency: public ConfigObject
{
  Q_OBJECT

  Q_CLASSINFO("IdPrefix", "af")

  Q_CLASSINFO("frequencyDecription",
              "Transmit-frequency.")

  /** The frequency. */
  Q_PROPERTY(Frequency frequency READ frequency WRITE setFrequency)

public:
  /** Default constructor. */
  Q_INVOKABLE explicit AnytoneAPRSFrequency(QObject *parent=nullptr);

  ConfigItem *clone() const;

  /** Returns the transmit frequency. */
  Frequency frequency() const;
  /** Sets the transmit frequency. */
  void setFrequency(Frequency freq);

protected:
  /** The transmit frequency. */
  Frequency _frequency;
};


/** Represents a reference to an APRS frequency.
 * @ingroup anytone */
class AnytoneAPRSFrequencyRef: public ConfigObjectReference
{
  Q_OBJECT

public:
  /** Default constructor. */
  explicit AnytoneAPRSFrequencyRef(QObject *parent=nullptr);
};


/** Represents a list of APRS transmit frequencies.
 * @ingroup anytone */
class AnytoneAPRSFrequencyList: public ConfigObjectList
{
  Q_OBJECT

public:
  /** Empty constructor. */
  explicit AnytoneAPRSFrequencyList(QObject *parent=nullptr);

  ConfigItem *allocateChild(const YAML::Node &node, ConfigItem::Context &ctx, const ErrorStack &err);
};


/** Implements the common properties for analog and digital AnyTone channels.
 * This class cannot be instantiated directly, use one of the derived classes.
 * @ingroup anytone */
class AnytoneChannelExtension: public ConfigExtension
{
  Q_OBJECT

  /** Holds the frequency correction in some unknown units. */
  Q_PROPERTY(int frequencyCorrection READ frequencyCorrection WRITE setFrequencyCorrection)
  /** If @c true, the hands-free feature is enabled for this channel. */
  Q_PROPERTY(bool handsFree READ handsFree WRITE enableHandsFree)
  /** A reference to the FM APRS frequency. If not set, the default will be used. */
  Q_PROPERTY(AnytoneAPRSFrequencyRef *fmAPRSFrequency READ fmAPRSFrequency())
  /** Specifies if and when the position is send via the associated APRS system, once the PTT is
   * pressed. */
  Q_PROPERTY(APRSPTT aprsPTT READ aprsPTT WRITE setAPRSPTT)
  /** If @c true, the talker alias is sent on this channel (per-channel flag of the D878UV and
   * newer, "Send Talker Alias" in the manufacturer CPS). */
  Q_PROPERTY(bool sendTalkerAlias READ sendTalkerAlias WRITE enableSendTalkerAlias)
  /** If @c true, the radio starts scanning automatically when this channel is selected
   * ("Auto Scan" in the manufacturer CPS). */
  Q_PROPERTY(bool autoScan READ autoScan WRITE enableAutoScan)
  /** If @c true, the FM APRS reception is muted on this channel ("Ana Aprs Mute" in the
   * manufacturer CPS). */
  Q_PROPERTY(bool fmAPRSMute READ fmAPRSMute WRITE enableFMAPRSMute)
  /** Additional scan lists of this channel. Some devices (e.g., DMR-6X2UV) assign up to 8 scan
   * lists to a channel, the first one is the channel's scan list. */
  Q_PROPERTY(ScanListRefList *additionalScanLists READ additionalScanLists)

public:
  /** Possible APRS PTT modes. */
  enum class APRSPTT{
    Off, Start, End
  };
  Q_ENUM(APRSPTT)

protected:
  /** Hidden constructor. */
  explicit AnytoneChannelExtension(QObject *parent=nullptr);

public:
  /** Returns the frequency correction in some unknown units. */
  int frequencyCorrection() const;
  /** Sets the frequency correction. */
  void setFrequencyCorrection(int corr);

  /** Returns @c true if the hands-free feature is enabled. */
  bool handsFree() const;
  /** Enables/disables the hands-free feature for this channel. */
  void enableHandsFree(bool enable);

  /** Holds a reference to the FM APRS frequency to be used if FM APRS is enabled on the channel. */
  AnytoneAPRSFrequencyRef *fmAPRSFrequency() const;

  /** Holds the APRS PTT mode. That his, if and when the APRS information is send via the
   * associated APRS system. */
  APRSPTT aprsPTT() const;
  /** Sets the APRS PTT mode. */
  void setAPRSPTT(APRSPTT mode);

  /** Returns @c true if the talker alias is sent on this channel. */
  bool sendTalkerAlias() const;
  /** Enables/disables sending the talker alias on this channel. */
  void enableSendTalkerAlias(bool enable);

  /** Returns @c true if auto scan is enabled for this channel. */
  bool autoScan() const;
  /** Enables/disables auto scan for this channel. */
  void enableAutoScan(bool enable);

  /** Returns @c true if the FM APRS reception is muted on this channel. */
  bool fmAPRSMute() const;
  /** Enables/disables muting of the FM APRS reception on this channel. */
  void enableFMAPRSMute(bool enable);

  /** Returns the list of additional scan lists. */
  ScanListRefList *additionalScanLists() const;

protected:
  /** The frequency correction. */
  int _frequencyCorrection;
  /** If @c true, the hands-free feature is enabled for this channel. */
  bool _handsFree;
  /** A reference to the FM APRS frequency. */
  AnytoneAPRSFrequencyRef *_fmAPRSFrequency;
  /** Holds the APRS PTT mode. */
  APRSPTT _aprsPTT;
  /** If @c true, the talker alias is sent on this channel. */
  bool _sendTalkerAlias;
  /** If @c true, auto scan is enabled for this channel. */
  bool _autoScan;
  /** If @c true, the FM APRS reception is muted on this channel. */
  bool _fmAPRSMute;
  /** The additional scan lists. */
  ScanListRefList *_additionalScanLists;
};


/** Implements the settings extension for FM channels on AnyTone devices.
 * @ingroup anytone */
class AnytoneFMChannelExtension: public AnytoneChannelExtension
{
  Q_OBJECT

  /** If @c true, the custom CTCSS tone is used for RX (open squelch). */
  Q_PROPERTY(bool rxCustomCTCSS READ rxCustomCTCSS WRITE enableRXCustomCTCSS)
  /** If @c true, the custom CTCSS tone is transmitted. */
  Q_PROPERTY(bool txCustomCTCSS READ txCustomCTCSS WRITE enableTXCustomCTCSS)
  /** Holds the custom CTCSS tone frequency in Hz. Resolution is 0.1Hz */
  Q_PROPERTY(double customCTCSS READ customCTCSS WRITE setCustomCTCSS)
  /** Holds the squelch mode. */
  Q_PROPERTY(SquelchMode squelchMode READ squelchMode WRITE setSquelchMode)
  /** If @c true, the analog scrabler is enabled. */
  Q_PROPERTY(Frequency scramblerFrequency READ scramblerFrequency WRITE setScramblerFrequency)
  /** If @c true, the CTCSS phase reversal (reverse burst) is enabled ("Reverse" in the
   * manufacturer CPS). */
  Q_PROPERTY(bool ctcssPhaseReversal READ ctcssPhaseReversal WRITE enableCTCSSPhaseReversal)
  /** Specifies when the PTT ID is sent. */
  Q_PROPERTY(PTTId pttID READ pttID WRITE setPTTID)
  /** Specifies the optional signaling used on this channel. */
  Q_PROPERTY(OptionalSignaling optionalSignaling READ optionalSignaling WRITE setOptionalSignaling)
  /** Index of the DTMF ID (encode list) used on this channel, 0-based. */
  Q_PROPERTY(unsigned int dtmfID READ dtmfID WRITE setDTMFID)
  /** Index of the 2-tone ID (encode list) used on this channel, 0-based. */
  Q_PROPERTY(unsigned int twoToneID READ twoToneID WRITE setTwoToneID)
  /** Index of the 5-tone ID (encode list) used on this channel, 0-based. */
  Q_PROPERTY(unsigned int fiveToneID READ fiveToneID WRITE setFiveToneID)
  /** Index of the 2-tone decode entry used on this channel, 0-based. */
  Q_PROPERTY(unsigned int twoToneDecode READ twoToneDecode WRITE setTwoToneDecode)
  /** Index of the 5-tone ID sent at the begin of a transmission (BOT), 0-based. */
  Q_PROPERTY(unsigned int fiveToneBOT READ fiveToneBOT WRITE setFiveToneBOT)
  /** Index of the 5-tone ID sent at the end of a transmission (EOT), 0-based. */
  Q_PROPERTY(unsigned int fiveToneEOT READ fiveToneEOT WRITE setFiveToneEOT)

public:
  /** Possible squelch mode settings. */
  enum class SquelchMode {
    Carrier = 0,
    SubTone = 1,
    OptSig  = 2,
    SubToneAndOptSig = 3,
    SubToneOrOptSig = 4
  };
  Q_ENUM(SquelchMode)

  /** Possible PTT ID settings. */
  enum class PTTId {
    Off = 0, Start = 1, End = 2, Both = 3
  };
  Q_ENUM(PTTId)

  /** Possible optional signaling settings. */
  enum class OptionalSignaling {
    Off = 0, DTMF = 1, TwoTone = 2, FiveTone = 3
  };
  Q_ENUM(OptionalSignaling)

public:
  /** Default constructor. */
  Q_INVOKABLE explicit AnytoneFMChannelExtension(QObject *parent=nullptr);

  ConfigItem *clone() const;

  /** Returns @c true, if the custom CTCSS frequency is used for RX (open squelch). */
  bool rxCustomCTCSS() const;
  /** Enables/disables usage of custom CTCSS frequency for RX. */
  void enableRXCustomCTCSS(bool enable);
  /** Returns @c true, if the custom CTCSS frequency is used for TX (open squelch). */
  bool txCustomCTCSS() const;
  /** Enables/disables usage of custom CTCSS frequency for TX. */
  void enableTXCustomCTCSS(bool enable);
  /** Returns the custom CTCSS frequency in Hz. Resolution is 0.1Hz. */
  double customCTCSS() const;
  /** Sets the custom CTCSS frequency in Hz. Resolution is 0.1Hz. */
  void setCustomCTCSS(double freq);

  /** Returns the squelch mode. */
  SquelchMode squelchMode() const;
  /** Sets the squelch mode. */
  void setSquelchMode(SquelchMode mode);

  /** Reuturns the FM scrabler carrier frequency. */
  Frequency scramblerFrequency() const;
  /** Sets the FM scrambler carrier frequency. */
  void setScramblerFrequency(const Frequency &freq);

  /** Returns @c true if the CTCSS phase reversal is enabled. */
  bool ctcssPhaseReversal() const;
  /** Enables/disables the CTCSS phase reversal. */
  void enableCTCSSPhaseReversal(bool enable);

  /** Returns the PTT ID setting. */
  PTTId pttID() const;
  /** Sets the PTT ID setting. */
  void setPTTID(PTTId mode);

  /** Returns the optional signaling. */
  OptionalSignaling optionalSignaling() const;
  /** Sets the optional signaling. */
  void setOptionalSignaling(OptionalSignaling sig);

  /** Returns the DTMF ID index. */
  unsigned int dtmfID() const;
  /** Sets the DTMF ID index. */
  void setDTMFID(unsigned int idx);
  /** Returns the 2-tone ID index. */
  unsigned int twoToneID() const;
  /** Sets the 2-tone ID index. */
  void setTwoToneID(unsigned int idx);
  /** Returns the 5-tone ID index. */
  unsigned int fiveToneID() const;
  /** Sets the 5-tone ID index. */
  void setFiveToneID(unsigned int idx);
  /** Returns the 2-tone decode index. */
  unsigned int twoToneDecode() const;
  /** Sets the 2-tone decode index. */
  void setTwoToneDecode(unsigned int idx);
  /** Returns the BOT 5-tone ID index. */
  unsigned int fiveToneBOT() const;
  /** Sets the BOT 5-tone ID index. */
  void setFiveToneBOT(unsigned int idx);
  /** Returns the EOT 5-tone ID index. */
  unsigned int fiveToneEOT() const;
  /** Sets the EOT 5-tone ID index. */
  void setFiveToneEOT(unsigned int idx);

protected:
  /** If @c true, the custom CTCSS tone is used for RX (open squelch). */
  bool _rxCustomCTCSS;
  /** If @c true, the custom CTCSS tone is transmitted. */
  bool _txCustomCTCSS;
  /** Holds the custom CTCSS tone frequency in Hz. Resolution is 0.1Hz */
  double _customCTCSS;
  /** Holds the squelch mode. */
  SquelchMode _squelchMode;
  /** Sets the FM scrambler frequency. */
  Frequency _scramblerFrequency;
  /** If @c true, the CTCSS phase reversal is enabled. */
  bool _ctcssPhaseReversal;
  /** The PTT ID setting. */
  PTTId _pttID;
  /** The optional signaling. */
  OptionalSignaling _optionalSignaling;
  /** DTMF ID index. */
  unsigned int _dtmfID;
  /** 2-tone ID index. */
  unsigned int _twoToneID;
  /** 5-tone ID index. */
  unsigned int _fiveToneID;
  /** 2-tone decode index. */
  unsigned int _twoToneDecode;
  /** BOT 5-tone ID index. */
  unsigned int _fiveToneBOT;
  /** EOT 5-tone ID index. */
  unsigned int _fiveToneEOT;
};


/** Implements the settings extension for DMR channels on AnyTone devices.
 * @ingroup anytone */
class AnytoneDMRChannelExtension: public AnytoneChannelExtension
{
  Q_OBJECT

  /** If @c true, the adaptive TDMA mode is enabled. This makes only sense, if @c simplexTDMA is
   * enabled too. In this case, the radio is able to receive both simplex TDMA as well as "normal"
   * simplex DMR on the channel. */
  Q_PROPERTY(bool adaptiveTDMA READ adaptiveTDMA WRITE enableAdaptiveTDMA)
  /** If @c true, the through mode is enabled (what ever that means). */
  Q_PROPERTY(bool throughMode READ throughMode WRITE enableThroughMode)
  /** If @c true, the DMR CRC check is enabled (default). */
  Q_PROPERTY(bool crc READ crcEnabled WRITE enableCRC);
  /** If @c true, ranging is enabled on this channel. */
  Q_PROPERTY(bool ranging READ ranging WRITE enableRanging)

public:
  /** Default constructor. */
  Q_INVOKABLE explicit AnytoneDMRChannelExtension(QObject *parent=nullptr);

  ConfigItem *clone() const;

  /** Returns @c true if the adaptive TDMA mode is enabled. */
  bool adaptiveTDMA() const;
  /** Enables/disables the adaptive TDMA mode. */
  void enableAdaptiveTDMA(bool enable);
  /** Returns @c true if the through mode is enabled. */
  bool throughMode() const;
  /** Enables/disables the through mode. */
  void enableThroughMode(bool enable);
  /** Returns @c true if the DMR CRC check is enabled (default). */
  bool crcEnabled() const;
  /** Enables DMR CRC check. */
  void enableCRC(bool enable);
  /** Returns @c true if ranging is enabled. */
  bool ranging() const;
  /** Enables/disables ranging. */
  void enableRanging(bool enable);

protected:
  /** If @c true, the adaptive TDMA mode is enabled. */
  bool _adaptiveTDMA;
  /** If @c true the through mode is enabled. */
  bool _throughMode;
  /** If @c true, DMR CRC check is enabled. */
  bool _crc;
  /** If @c true, ranging is enabled. */
  bool _ranging;
};


/** Implements the AnyTone extensions for zones.
 * @ingroup anytone */
class AnytoneZoneExtension : public ConfigExtension
{
  Q_OBJECT

  /** If @c true, the zone is hidden in the menu. */
  Q_PROPERTY(bool hidden READ hidden WRITE enableHidden)

public:
  /** Default constructor. */
  Q_INVOKABLE explicit AnytoneZoneExtension(QObject *parent=nullptr);

  ConfigItem *clone() const;

  /** Returns @c true if the zone is hidden. */
  bool hidden() const;
  /** Enables/disables hidden zone. */
  void enableHidden(bool enable);

protected:
  /** If @c true, the zone is hidden in the menu. */
  bool _hidden;
};


/** Implements the AnyTone contact extension.
 * @ingroup anytone */
class AnytoneContactExtension: public ConfigExtension
{
  Q_OBJECT

  /** Overrides the ring flag, allows to set None, Ring and Online. */
  Q_PROPERTY(AlertType alertType READ alertType WRITE setAlertType)

public:
  /** Possible ring-tone types. */
  enum class AlertType {
    None = 0,                   ///< Alert disabled.
    Ring = 1,                   ///< Ring tone.
    Online = 2                  ///< WTF?
  };
  Q_ENUM(AlertType)

public:
  /** Default constructor. */
  Q_INVOKABLE explicit AnytoneContactExtension(QObject *parent=nullptr);

  ConfigItem *clone() const;

  /** Returns the alert type for the contact. */
  AlertType alertType() const;
  /** Sets the alert type for the contact. */
  void setAlertType(AlertType type);

protected:
  /** Holds the alert type for the contact. */
  AlertType _alertType;
};


/** Implements some additional settings for the FM APRS system.
 * This extension gets attached to a @c APRSSystem instance.
 */
class AnytoneFMAPRSSettingsExtension: public ConfigExtension
{
  Q_OBJECT

  /** The transmit delay in milliseconds. */
  Q_PROPERTY(Interval txDelay READ txDelay WRITE setTXDelay)
  /** The transmit pre-wave delay in milliseconds. */
  Q_PROPERTY(Interval preWaveDelay READ preWaveDelay WRITE setPreWaveDelay)
  /** If @c true, all APRS messages are processed, including those with invalid CRC. */
  Q_PROPERTY(bool passAll READ passAll WRITE enablePassAll)
  /** If @c true, the report position flag is set. */
  Q_PROPERTY(bool reportPosition READ reportPosition WRITE enableReportPosition)
  /** If @c true, the report Mic-E flag is set. */
  Q_PROPERTY(bool reportMicE READ reportMicE WRITE enableReportMicE)
  /** If @c true, the report object flag is set. */
  Q_PROPERTY(bool reportObject READ reportObject WRITE enableReportObject)
  /** If @c true, the report item flag is set. */
  Q_PROPERTY(bool reportItem READ reportItem WRITE enableReportItem)
  /** If @c true, the report message flag is set. */
  Q_PROPERTY(bool reportMessage READ reportMessage WRITE enableReportMessage)
  /** If @c true, the report weather flag is set. */
  Q_PROPERTY(bool reportWeather READ reportWeather WRITE enableReportWeather)
  /** If @c true, the report NMEA flag is set. */
  Q_PROPERTY(bool reportNMEA READ reportNMEA WRITE enableReportNMEA)
  /** If @c true, the report status flag is set. */
  Q_PROPERTY(bool reportStatus READ reportStatus WRITE enableReportStatus)
  /** If @c true, the report other flag is set. */
  Q_PROPERTY(bool reportOther READ reportOther WRITE enableReportOther)

  /** The list of additional APRS frequencies. */
  Q_PROPERTY(AnytoneAPRSFrequencyList *frequencies READ frequencies)

public:
  /** Possible bandwidth settings. */
  enum class Bandwidth {
    Narrow = 0, Wide = 1
  };
  Q_ENUM(Bandwidth)

public:
  /** Default constructor. */
  explicit Q_INVOKABLE AnytoneFMAPRSSettingsExtension(QObject *parent=nullptr);

  ConfigItem *clone() const;

  /** Returns the transmit delay. */
  Interval txDelay() const;
  /** Sets the transmit delay. */
  void setTXDelay(Interval intv);

  /** Returns the pre-wave delay in ms. */
  Interval preWaveDelay() const;
  /** Sets the pre-wave delay in ms. */
  void setPreWaveDelay(Interval ms);

  /** Returns @c true if all received APRS messages are processed, even those with invalid CRC. */
  bool passAll() const;
  /** Enables processing of all received APRS messages, including those with invalid CRC. */
  void enablePassAll(bool enable);

  /** Returns @c true if the report position flag is set. */
  bool reportPosition() const;
  /** Enables/disables report position flag. */
  void enableReportPosition(bool enable);
  /** Returns @c true if the report Mic-E flag is set. */
  bool reportMicE() const;
  /** Enables/disables report Mic-E flag. */
  void enableReportMicE(bool enable);
  /** Returns @c true if the report object flag is set. */
  bool reportObject() const;
  /** Enables/disables report object flag. */
  void enableReportObject(bool enable);
  /** Returns @c true if the report item flag is set. */
  bool reportItem() const;
  /** Enables/disables report item flag. */
  void enableReportItem(bool enable);
  /** Returns @c true if the report message flag is set. */
  bool reportMessage() const;
  /** Enables/disables report message flag. */
  void enableReportMessage(bool enable);
  /** Returns @c true if the report weather flag is set. */
  bool reportWeather() const;
  /** Enables/disables report weather flag. */
  void enableReportWeather(bool enable);
  /** Returns @c true if the report NMEA flag is set. */
  bool reportNMEA() const;
  /** Enables/disables report NMEA flag. */
  void enableReportNMEA(bool enable);
  /** Returns @c true if the report status flag is set. */
  bool reportStatus() const;
  /** Enables/disables report status flag. */
  void enableReportStatus(bool enable);
  /** Returns @c true if the report other flag is set. */
  bool reportOther() const;
  /** Enables/disables report other flag. */
  void enableReportOther(bool enable);

  /** Returns the list of additional FM APRS frequencies. */
  AnytoneAPRSFrequencyList *frequencies() const;

protected:
  /** The transmit delay. */
  Interval _txDelay;
  /** The pre-wave delay. */
  Interval _preWaveDelay;
  /** If @c true, all APRS messages are processed. */
  bool _passAll;
  /** If @c true the report position flag is set. */
  bool _reportPosition;
  /** The report Mic-E flag. */
  bool _reportMicE;
  /** The report object flag. */
  bool _reportObject;
  /** The report item flag. */
  bool _reportItem;
  /** The report message flag. */
  bool _reportMessage;
  /** The report weather flag. */
  bool _reportWeather;
  /** The report NMEA flag. */
  bool _reportNMEA;
  /** The report status flag. */
  bool _reportStatus;
  /** The report other flag. */
  bool _reportOther;
  /** The list of additional FM APRS frequencies. */
  AnytoneAPRSFrequencyList *_frequencies;
};

#endif // ANYTONEEXTENSION_HH
