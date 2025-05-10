let S = \x.\y.\z.x z(y z)
let K = \x.\y.x
let I = \x.x

let Y = \f.(\x.x x) (\x.f(x x))

let list = pair "1" (pair "2" (pair "3" nil))

let main = Y (\self.\xs.(is_empty xs) nil ("n" (head xs) self (tail xs))) list

let pair = \x.\y.\b.b x y
let head = \p.p (\x.\y.x)
let tail = \p.p (\x.\y.y)
let is_empty = \xs.xs (\h.\t.\d.nil) (\x.\y.x)
let nil = \x.\y.y
