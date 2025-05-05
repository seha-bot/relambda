let S = \x.\y.\z.x z(y z)
let K = \x.\y.x
let I = \x.x

let Y = \f.(\x.x x) (\x.f(x x))

let list = pair nil (pair nil (pair nil nil))

let main = Y (\self.\xs.(is_empty xs) nil (self (tail xs))) list
let main1 = Y (K I)
let main2 = Y

let pair = \x.\y.\b.b x y
let tail = \p.p (\x.\y.y)
let is_empty = \xs.xs (\h.\t.\d.nil) (\x.\y.x)
let nil = \x.\y.y
