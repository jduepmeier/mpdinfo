mpdinfo
=======

show mpd information (for status displays, ...). Signal driven.

requirements
=======

* libmpdclient

configuration
=======
MPDHOST sets the host

MPDPORT sets the port

You can use a config file, too. See [a relative link](sample.conf) for more info.
mpdinfo only loads the config with --config <path-to-config>.

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
