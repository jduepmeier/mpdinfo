mpdinfo
=======
Shows metadata like title, artist, ... Highly configurable over a simple config file. Works best with tools like (sm or [xecho](https://github.com/cbdevnet/xecho)) to display on a status display. It can be controlled via signals.

requirements
=======

* libmpdclient

configuration
=======
* MPD_HOST sets the host
* MPD_PORT sets the port

You can use a config file, too. See [sample.conf](sample.conf) for more info.
mpdinfo only loads the config with `--config <path-to-config>`.

use
=======
mpdinfo waits for events from mpd. On every event all infos will be printed.
At the end of every output, mpdinfo sends a formfeed (\f).
It works nicely with screen-message (sm).

signals
=======
You can send signal to control mpdinfo.
* SIGHUP forces a refresh
* SIGQUIT quits mpdinfo
