# MMC5983MA Compass Test - Quick-And-Dirty test showing field magnitude measurement problems
Either I'm doing something wrong (hardly ever happens), or the MMC5983MA is not working properly!
I'd be delighted if anyone can report success with the MMC5983MA and show me where I've made a mistake!

Rotating the MMC5983MA at a fixed point in space, it's measuring different field magnitudes;
the magnitude should be constant.  Note magnitude is sqrt(X^2+Y^2+Z^2), and reported values range from 
131% down to 76% of the expected local field strength for my location (from WMM World Magnetic Model).
So in the right ballpark and making measurements, but definitely something is wrong.
[The file ExampleMagnitudeProblemLog.txt](ExampleMagnitudeProblemLog.txt) shows details of the erroneous readings.

I'm following the datasheet here: https://www.memsic.com/Public/Uploads/uploadfile/files/20220119/MMC5983MADatasheetRevA.pdf
This device has a SET/RESET function which gives a big pulse and resets the device,
clearing any residual and setting the measurement polarity (reverses for SET vs RESET). 
Per the datasheet, I'm making a RESET, measuring, a SET, measuring.
Then calculate the center-value (nominal zero-field output which MEMSIC calls _offset_) and measurement given two readings with opposite polarity.

Test program is a C++ 32-bit Windows program using wxWidgets, and communicating with the SparkFun
Qwiic MMC5983A board using an Adafruit MCP2221 USB-to-Qwiic adapter.
Included is the MMC5983A C++ device driver I wrote.

# The datasheet has a number of issues:
Datasheet Questions (MEMSIC MMC5983MA Rev A -- Formal release date: 4/3/2019)
The datasheet is seriously unclear on a number of points,
and sometimes conflicts with MEMSIC's sample code.
Unfortunately, I've had great difficulty getting sensible answers from MEMSIC technical support.

1) Continuous mode vs. one-shot mode aren't well documented.
   - a) Is it necessary to write TM_M to start measurements in continuous mode?
   - b) In continuous mode, is Meas_M_Done ever set?
   - c) Are TM_M and Meas_M_Done only for one-shot measurements?
   - d) Does temperature measurement require continuous pressure mode to be turned off?
      ANSWERED SORT-OF: **Temperature sensor does not really work, don't use it.**
2) Can SET and RESET used while continuous mode is enabled?
3) The use and behavior of auto-SET-RESET is not documented:
   - a) What is the PURPOSE of auto-SET-RESET?
	  Unless a large field saturates the sensor, why is this helpful?
	  How can this improve performance?
   - b) The Auto_SR_en bit is labeled "Automatic Set-Reset".
	  What exactly does the sensor do if this bit is set?
	  Does it operate in both continuous and one-shot mode?
   - c) The sample code provided by MEMSIC says "continuous mode with auto set and reset" - is that correct?
	  The datasheet page 16 says "The device will perform a SET automatically...";
	  should that be "performs a periodic RESET/SET sequence"?
	  Which is the correct explanation of what the chip does?
	  The order of SET and RESET determines the sensing polarity, so this needs to be clear, right?
4) Explain saturation detection - what are the steps?
   What is the additional applied saturation test field in Gauss when a saturation-enable bit is set?
   Is it necessary to set the enable bits back to 0, or do they reset automatically like SET/RESET?
5) The datasheet says SET and RESET operations take 500 nsec.
   MEMSIC sample code waits 500 usec. Which is correct?
6) The decimation filter is mentioned but not described.
   Given BW and CM settings, What *exactly* is the sampling and filter?
7) OTP read controls are discussed, but OTP is never defined.
   What is it? Why do we care? 
   ANSWERED: OTP indicates MMC5983MA's one-time programming information was
   read successfully by firmware after the last reset. **This bit should always be set.**
