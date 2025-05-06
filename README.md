# relambda

steps:

1. parse to ast
2. verify ast
3. convert to ski
4. simplify
5. format to unlambda


unlambda Y: ``s`k``sii``s``s`ksk`k``s`kd``sii

relambda S nije isto sto i unlambda S:
(\x.\y.\z.x z (D (y z))) (KI) (D print) I
\x.D (F x) != D F

Are there two kinds of D? I think that the Ds which originate from preprocessing can't be optimized out, but there can be optimization strategies for Ds which originate from subsequent transformations. Verify this.