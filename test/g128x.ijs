1:@:(dbr bind Debug)@:(9!:19)2^_44[(echo^:ECHOFILENAME) './g128x.ijs'
NB. 128!:0 and 128!:1 ---------------------------------------------------

a  =. ?19 4$100
qr =. 128!:0 a
q =. >0{qr
r =. >1{qr

($q) -: $a
($r) -: 2$1{$a

i =. =i.#r
1e_13>>./|,(a+0=a)%~a-q+/ . *r
1e_14>>./|,i - (|:q) +/ . * q
*./,(|*r)<:<:/~i.#r

s =. 128!:1 r
($r) -: $s
1e_14>>./|,i-r +/ . * s

a  =. j.&?~ 9 5$100
qr =. 128!:0 a
q =. >0{qr
r =. >1{qr

($q) -: $a
($r) -: 2$1{$a

i =. =i.#r
1e_13>>./|,(a+0=a)%~a-q+/ . * r
1e_14>>./|,i - (+|:q) +/ . * q
*./,(|*r)<:<:/~i.#r

s =. 128!:1 r
($r) -: $s
1e_14>>./|,i-r +/ . * s

'domain error' -: ex '128!:88'


NB. 128!:2 --------------------------------------------------------------

(+/   x) -: '+/'   128!:2 x=: ?3 4 5$1e9
(+/"1 x) -: '+/"1' 128!:2 x

NB. 128!:10 -------------------------------------------------------------

{{
todiag =. ([`(,.~@i.@#@])`])}
lrtoa =. (((1. todiag *) +/ . * (* -.)) >/~@i.@#)
assert. (-: 1&{::@(128!:10)@:lrtoa) 3 3 $ 2 _1 _2 _2 4 _1 _2 _1 3
assert. (-: 1&{::@(128!:10)@:lrtoa) 4 4  $ 2 _1 _2 5  _2 4 _1 3  _2 _1 3 2  2 1 _2 3
NB. ((>./@:,@:-) 1&{::@(128!:10)@:lrtoa)@:>:@:i.@:(,~)"0 i. 20
assert. 1e_15 > >./ ((>./@:,@:-) 1&{::@(128!:10)@:lrtoa)@:(1.&todiag)@:(0.01&*)@:(0 ?@$~ ,~)"0 i. 25
assert. 1e_14 > >./ ((>./@:,@:-) 1&{::@(128!:10)@:lrtoa)@:(1.&todiag)@:(0.01&*)@:(0 ?@$~ ,~)"0 ] 500 + i. 25
NB. assert. 1e_14 > >/. ((>./@:,@:-) 1&{::@(128!:10)@:lrtoa)@:(0 ?@$~ ,~)"0 ] 500 + i. 25
NB. sparse matrices
assert. 1e_15 > >./ ((>./@:,@:-) 1&{::@(128!:10)@:lrtoa)@:((1. todiag (2#[) $ (0.01 * ?@$&0@])`((? *:)~)`(0. #~ *:@[)})   [: <. 0.01 * *:)"0 i. 25
assert. 1e_15 > >./ ((>./@:,@:-) 1&{::@(128!:10)@:lrtoa)@:((1. todiag (2#[) $ (0.01 * ?@$&0@])`((? *:)~)`(0. #~ *:@[)})   [: <. 0.001 * *:)"0 ] 500 + i. 25
ckmat =. {{ if. 1e_10 < err =. x (>./@:,@:- 1&{::@(128!:10)) y do. 13!:8 (4) ] errx =: x [ erry =: y end. err }}
assert. 1e_14 > >./ (ckmat lrtoa)@:((1. todiag (2#[) $ (0.01 * ?@$&0@])`((? *:)~)`(0. #~ *:@[)})   [: <. 0.01 * *:)"0 ] 500 + i. 50
assert. 1e_14 > >./ (ckmat lrtoa)@:((1. todiag (2#[) $ (0.01 * ?@$&0@])`((? *:)~)`(0. #~ *:@[)})   [: <. 0.1 * *:)"0 ] 500 + i. 50
assert. 1e_14 > >./ (ckmat lrtoa)@:((1. todiag (2#[) $ (0.01 * ?@$&0@])`((? *:)~)`(0. #~ *:@[)})   [: <. 0.001 * *:)"0 ] 500 + i. 50
1
}}^:(1 e. 'avx2' E. 9!:14'') 1
NB. LU rational
todiag =. ([`(,.~@i.@#@])`])}  NB. stuff x into diagonal of y
lrtoa =. (((1 todiag *) +/ . * (* -.)) >/~@i.@#)  NB. y is compressed Doolittle form, result is original a
(-: (0&{:: /:~ lrtoa@(1&{::))@(128!:10))@(1000x ?@$~ ,~)"0 i. 15


1: 0 : 0
sm =. ((1. todiag (2#[) $ (0.01 * ?@$&0@])`((? *:)~)`(0. #~ *:@[)})   [: <. 0.001 * *:) 1000
dm =.lrtoa@:(1.&todiag)@:(0.01&*)@:(0 ?@$~ ,~) 1000
)


4!:55 ;:'a i q qr r todiag lrtoa s x'


