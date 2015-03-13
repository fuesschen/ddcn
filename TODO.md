# Introduction #

There are quite some features that would be nice to have but can not be implemented in the short timeframe of the programming contest in which this project is built. And there are even more things which might help ddcn but have to be thought through and tried first. This is a list of these.

# TODO #

Performance:
  * Initially, switching between several modes was planned: No encryption, signature only and full encryption. While full encryption is implemented, the other two are not. Especially using only digital signatures might be interesting because for large files this might be quite a bit faster than full encryption.

Other things:
  * Revoking group keys: What to do if a member of your trusted group was compromised? At that time the attacker has the private key of the group. One might want some way to let all other network members that the old key is now invalid.
  * Log messages (currently Log messages always go to stdout)