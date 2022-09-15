# BS1770gain (Minus the Garbage)

## BS1770gain (The Original)

BS1770gain is a command-line loudness scanner and normalizer compliant with
ITU-R BS.1770 and its flavors EBU R128, ATSC A/85, and ReplayGain 2.0. It
helps normalizing the loudness of audio and video files to the same level.
Mostly simple, mostly straightforward, but definitely useful, given
that almost every service making use of audio these days requires
some type of normalization based on BS.1770.

## BS1770gain (The Garbage)

The original BS1770gain was written by someone whom I will (charitably)
call "human garbage". The person (who shall not name) is a blatant racist
with a rather problematic world view towards anyone not of their heritage.
Normally this wouldn't be a *massive* problem (because people are entitled
to their personal beliefs, no matter how abhorrent), but this person decided
to start putting racist messages throughout the documentation, source code,
and even compiled binary (including user-visible command output) in this
otherwise useful package.

People shouldn't have to tolerate that kind of garbage just to work with a
piece of software.

Normally I'd just use a different bit of software that does much the same
thing, but best I can tell, BS1770gain doesn't actually have a good
replacement in the open source world. `ffmpeg` can kind of manage it, but
with far less configurability (and I *think* it only does the old R.128
standard as well, though that's unclear). So that is (as far as I know)
not really an option. Someone please let me know if I'm wrong?

So instead, through the magic of open source, I have created this "fork"
of the BS1770gain software, with the primary change in the fork being
"the removal of all traces of the original author and their racist messages".
I've gone through and collected every version of the source code that I can
find, run some (bad) scripting to strip out all of the crap, and build this
repository out of the results, complete with version history.

This means that it should be possible to work with this source code without
ever encountering any of the garbage that was in the original package, or
even a mention of the original developer's name. I hated doing this, but
*damn*.

## The Future

It doesn't seem like anyone really uses this package anymore (between a
combination of most commercial software having something similar built in
to its GUI at this point, and the package getting dropped from many
distributions because of all this problematic garbage), but if people report
problems or submit PRs, I will do my best to fix/merge them and at least keep
the software functional. I suspect I'll really be the only user, but you never
know...

I would also appreciate reports of anywhere I missed any of the original
developer's garbage messaging.

--A
