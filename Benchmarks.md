# Introduction #

DDCNs main goal is to speed up compilation especially on slower machines - but there are other requirements like decentral structure and encryption which hinder this goal.

We tested mainly on three computers:
  * `eee`: Asus Eee PC '1005PX' (Netbook), Intel Atom N450, 1 x 1.66 GHz + HT
  * `r530`: Samsung `R530` laptop, Intel Pentium T4400, 2 x 2.2 GHz
  * `r580`: Samsung `R580` laptop, Intel Core i5 M530, 2 x 2.4GHz + HT

All tests were done over a simple 54Mb wireless connection. What we did was to compile ddcn through itself - this results in huge preprocessed files because of Qt, so compression was enabled.

# Numbers #

Compiling on eee:
  * `eee` alone: 2m 37s
  * `eee` + `r580`: 1m 12s (48%)

Note how the small system does indeed get a strong speedup - not nearly as good as it could be in theory though as it is by far the slower machine of the two.

Compiling on `r530`:
  * `r530` alone, without ddcn: 42s
  * `r530` alone, with ddcn: 44s (104%)
  * `r530` + `r580`: 22s (52%)

This shows that the overhead of ddcn is usually unnoticable.

TODO: Results with 3 or more machines (these so far have only been made with compression disabled which skews the numbers a lot)