/*
 * Copyright (c) 2021  Moddable Tech, Inc.
 *
 *   This file is part of the Moddable SDK.
 *
 *   This work is licensed under the
 *       Creative Commons Attribution 4.0 International License.
 *   To view a copy of this license, visit
 *       <http://creativecommons.org/licenses/by/4.0>.
 *   or send a letter to Creative Commons, PO Box 1866,
 *   Mountain View, CA 94042, USA.
 *
 */

import Timer from "timer";
import RTC from "embedded:peripherals/RTC-NXP/PCF8563";
import config from "mc/config";

trace(`Today's date: ${(new Date()).toGMTString()}\n`);


const rtc = new RTC ({
			...device.I2C.default,
			io: device.io.SMBus,
			interrupt: {
				io: device.io.Digital,
				pin: config.AlarmPin
			},
			onAlarm() {
				this.iter = (this.iter ? this.iter + 1 : 1);
				trace(`Alarm!! ${(new Date(rtc.time)).toGMTString()}\n`);
				switch (this.iter) {
					case 1:
						rtc.alarm = rtc.time + 2 * 60 * 1000;	// 2 minutes
						break;
					case 2:
						rtc.alarm = rtc.time + 5 * 60 * 1000;	// 5 minutes
						break;
					case 3:
						rtc.alarm = rtc.time + 60 * 60 * 1000;	// 1 hour
						break;
					case 4:
						rtc.alarm = rtc.time + 6 * 60 * 60 * 1000;	// 6 hours
						break;
					case 5:
						rtc.alarm = rtc.time + 12 * 60 * 60 * 1000;	// 12 hours
						break;
					case 6:
						rtc.alarm = rtc.time + 24 * 60 * 60 * 1000;	// 24 hours
						break;
				}
				trace(`set alarm to ${new Date(rtc.alarm).toGMTString()}\n`);
			},
			onError(v) { trace(v); }
});

if (!rtc.enabled) {
	trace(`CLOCK NOT ENABLED\n`);
	rtc.time = new Date();
}

/* To check month/year boundary, uncomment below
let now = new Date(rtc.time);
now.setUTCDate(31);
now.setUTCMonth(11);
now.setUTCHours(23);
now.setUTCMinutes(59);
trace(`set time to ${now.toGMTString()}\n`);
rtc.time = now;
*/

rtc.alarm = rtc.time + 60_000;	// 1 minute

Timer.delay(2000);
trace(`set alarm to ${new Date(rtc.alarm).toGMTString()}\n`);
