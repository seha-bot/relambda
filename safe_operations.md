# List of known safe operations which will be applied by the compiler

The compiler will attempt to simulate the "call by name" reduction strategy. That is, no reductions are performed inside abstractions and arguments are substituted into the body of an abstraction before the arguments are reduced.

## Lazyness preprocessing (unverified)

For each application `F G` in the lambda form, apply `D` to the right-hand side (`F (D G)`) if `G` is unsafe.

[Note 1: This will be applied as the first step in the compilation and forces lazy evaluation. -- end note]

[Note 2 (unverified): Local variables are always safe because the to-be-substituted expression will already be delayed elsewhere by this rule. -- end note]

[Note 3: Expressions in the form `D F` are always safe because evaluating them does nothing. -- end note]

[Example 1 (unverified for the same reason as Note 2):

`λx.x x` can remain as-is because the right-hand side of the application is a local variable.

-- end example]

[Example 2:
```
# We don't expect this to print anything,
# so `D` will be applied to the right-hand side
# to prevent eager evaluation in unlambda.
(λx.x) ("a" (\x.x))
# Preprocessed to (λx.x) (D ("a" (\x.x)))
```
-- end example]

## Transformations

After the preprocessing, the `T` function will be applied to the entire program. This function transforms the program according to the rules in the following sections.

Missing definitions: local variable, combinator, mention, expression, lambda form, evaluate.

In the following sections, `F` and `G` denote arbitary expressions.

An expression `F` is *safe* if evaluating it terminates and produces no side-effects. An expression `F` is *unsafe* if it is not safe.

[Note 1: It is not always possible to tell if an expression is safe, so compilers are allowed to treat any safe expression as unsafe. -- end note]

### Identity

An abstraction in the form `λx.x` shall be transformed to `I`.

Formula: \
`T (λx.x) = I`

### Constant expressions

An abstraction in the form `λx.F` where `F` does not mention `x` shall be transformed to:
- If `F` is safe, `K F`.
- Otherwise, `D (K F)`.

[Note 1 (unverified): Local variables which are different from `x` and combinators are considered safe, so it's ok to omit `D` for them. -- end note]

[Example 1:
```
# λy.λx.y = λy.K y = K

let print = # some side-effect

# We expect this to reduce to λx.print and to not print anything.
let main = (λy.λx.y) print # transformed to K (D print)
```

In the preprocessing stage, this will become `(λy.λx.y) (D print)`. Then, we can transform to: `(λy.K y) (D print)`. According to the "Applications in abstractions" transformation, we can further transform to `K (D print)`. If evaluated in unlambda, it will not print anything and has the same effect as the starting code.

-- end example]

[Note 2: The above example does not prove anything and just shows a case where it works. I don't know how to prove this. -- end note]

[Example 2:
```
let print = # some side-effect

# This is in normal form according to our reduction strategy and should not print anything.
let main = λx.print # transformed to D (K print)
```
-- end example]

Formula: \
`T (λx.F) = K F` if `F` doesn't mention `x` and `F` is safe \
`T (λx.F) = D (K F)` if `F` doesn't mention `x`

### Applications in abstractions

For an abstraction `A` in the form `λx.F G`:
- If `F` doesn't mention `x` and `G` is `x`:
    - If `F` is safe, `A` will be transformed to `T F`.
    - Otherwise, `A` will be transformed to `D (T F)`.
- Otherwise, `A` will be transformed to `S (T (λx.F)) (T (λx.G))`.

[Example 1:
```
let a = λx.x x # transforms to S I I
let b = λx.λy.x y # transforms to I
let c = λx."a" x # transforms to D "a"
```
-- end example]

Formula: \
`T (λx.F x) = T F` if `F` doesn't mention `x` and `F` is safe \
`T (λx.F x) = D (T F)` if `F` doesn't mention `x` and `F` is unsafe \
`T (λx.F G) = S (T (λx.F)) (T (λx.G))`\

### Applications

An application in the form `F G` shall be transformed to: `(T F) (T G)`.

Formula: \
`T (F G) = (T F) (T G)`

### Combinators and variables

An expression `F` shall be its own transformation if it is a combinator or a variable.

Formula: \
`T F = F` if `F` is a combinator or a variable
