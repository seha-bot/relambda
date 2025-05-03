let true = \x.\y.x
let false = \x.\y.y
let or = \lhs.\rhs.lhs true rhs
let and = \lhs.\rhs.lhs rhs false
let not = \x.x false true
let infix_or = true

let main = and (true infix_or((\x.false)false)) (not false)