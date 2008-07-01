
# This is a script to run a sequence of effects for a demo
# start a YARP server, then start ucanvcam.
# put it into "ParamTV" effect mode.
# Then run this script and hit return.

# The "PicMixTV" effects won't work since they rely on downloading
# effects from makesweet.com - you could download your own
# and play them.

echo hit return to start
read x
echo started

base="/home/paulfitz/cvs/build/vcam/linux-standalone"
pp=10
short=3

(
    echo "set PicMixTV"
    sleep $pp
    echo "set BrokenTV"
    sleep $short
    echo "set PicMixTV (dir $base/pm_flag)"
    sleep $pp
    echo "set lensTV"
    sleep $short
    echo "set PicMixTV (dir $base/pm_heart)"
    sleep $pp
    echo "set WarholTV"
    sleep $short
    echo "set TickerTV (text \"Happy Lizard ...... is Happy\")"
    sleep $pp
    echo "set AgingTV"
    sleep $short
    echo "set OverlayTV"
    sleep $short
    echo "set PicMixTV (dir $base/pm_wine)"
    sleep $pp
    echo "set PuzzleTV"
    sleep $short
    echo "set PicMixTV (dir $base/pm_card)"
    sleep $pp
    echo "set warpTV"
    sleep $short
    echo "set EngageTV"
    sleep $pp
) | yarp rpc /ctrl

