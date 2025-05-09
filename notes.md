# relambda

steps:

1. parse to ast
2. verify ast
3. convert to ski
4. simplify
5. format to unlambda


unlambda Y: ``s`k``sii``s``s`ksk`k``s`kd``sii
S (K(SII)) [ S (S(KS)K) (K(S(KD)(SII))) ]

relambda S nije isto sto i unlambda S:
(\x.\y.\z.x z (D (y z))) (KI) (D print) I
\x.D (F x) != D F

Are there two kinds of D? I think that the Ds which originate from preprocessing can't be optimized out, but there can be optimization strategies for Ds which originate from subsequent transformations. Verify this.

WARNING: this is a bad example cuz the variable substitution is currently faulty.
Case where `x y` (application of two variables) is pure:
```
\x.\y.\z.x y
\x.\y.K (x y)
\x.S (\y.K) (\y.x y)
\x.S (K K) x
S (K K)

S (K K) (D ("1" I)) (D ("2" I)) (D ("3" I))
K K (D ("2" I)) (D ("1" I) (D ("2" I))) (D ("3" I))
K (D ("1" I) (D ("2" I))) (D ("3" I))
K ("1" I (D ("2" I))) (D ("3" I))
K (I (D ("2" I))) (D ("3" I)) # prints 1
K (D ("2" I)) (D ("3" I))
D ("2" I)

(\x.\y.\z.x y) ("1" I) ("2" I) ("3" I)
(\y.\z."1" I y) ("2" I) ("3" I)
"1" I ("2" I)
I ("2" I) # prints 1
"2" I
I # prints 2
```

Case where `x y` (application of two variables) is unpure:
```
\f.\x.\y.f (x y)
\f.\x.S (K f) x
\f.S (K f)
S (K S) K

# unlambda evaluation
S (K S) K "1" "2" "3"
K S "1" (K "1") "2" "3"
S (K "1") "2" "3"
K "1" "3" ("2" "3")
"1" ("2" "3")
"1" "3" # 2 printed
"3" # 1 printed

# expected evaluation
(\f.\x.\y.f (x y)) "1" "2" "3"
(\x.\y."1" (x y)) "2" "3"
(\y."1" ("2" y)) "3"
"1" ("2" "3")
"2" "3" # 1 printed
"3" # 2 printed
```
This also applies if the two variables are the same name.
