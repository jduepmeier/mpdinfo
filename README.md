mpdinfo
=======

show mpd information (for status displays, ...). Signal driven.

requirements
=======

[*] libmpdclient

configuration
=======
MPDHOST sets the host
MPDPORT sets the port

use
=======
mpdinfo waits for events from mpd. On every event all infos will be printed.

signals
=======
You can send signal to control mpdinfo
[*] SIGHUP forces a refresh
[*] SIGQUIT quits mpdinfo
