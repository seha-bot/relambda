let zero = \f.\x.x
let one = inc zero
let two = inc one
let four = two two

let main = multiply four two

let inc = \num.\f.\x.f (num f x)
let add = \a.\b.a inc b
let multiply = \a.\b.b (add a) zero
let pow = \a.\b.b a
