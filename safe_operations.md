# List of known non-destructive operations applied by the compiler

The compiler will attempt to simulate the "call by name" reduction strategy. That is, no reductions are performed inside abstractions and arguments are substituted into the body of an abstraction before the arguments are reduced.

## Definitions for this document

A *definition* is a named expression introduced with `let`. An *abstraction* is a lambda (λ) followed by a name, a dot and an expression. A *variable* is a name which is introduced by an abstraction or a definition. A *combinator* is either `S`, or `K`, or `I` or `D`. An *expression* is either an abstraction, or a variable, or an application, or a combinator.

To *evaluate* an expression is to reduce it to its normal form according to the "call by name" reduction strategy.

Missing definitions: application, mention.

An expression `F` is *pure* if evaluating it terminates and produces no side-effects. An expression `F` is *unpure* if it is not pure.

[Note 1: It is not always possible to tell if an expression is pure, so compilers are allowed to treat any pure expression as unpure. There are some expressions which are [required](##-required-pure-expressions) to be treated as pure. -- end note]

## Lazyness preprocessing (unverified)

For each application `F G`, apply `D` to the right-hand side (`F (D G)`) if `G` is unpure.

[Note 1: This will be applied as the first step of the compilation and it forces lazy evaluation. -- end note]

[Example 1:

`λx.x x` can remain as-is because the right-hand side of the application is a variable.

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

After the preprocessing stage, the `T` function will be applied to the entire program. This function transforms the program according to the rules in the following sections. In the following sections, `F` and `G` denote arbitrary expressions.

### Identity

An abstraction in the form `λx.x` shall be transformed to `I`.

Formula: \
`T (λx.x) = I`

### Constant expression

An abstraction in the form `λx.F` where `F` does not mention `x` shall be transformed to:
- If `F` is pure, `K (T F)`.
- Otherwise, `D (K (T F))`.

[Example 1:
```
# λy.λx.y = λy.K y = K

let print = # some side-effect

# We expect this to reduce to λx.print and to not print anything.
let main = (λy.λx.y) print # transformed to K (D print)
```

In the preprocessing stage, this will become `(λy.λx.y) (D print)`. Then, we can transform to: `(λy.K y) (D print)`. According to the "Applications in abstractions" transformation, we can further transform to `K (D print)`. If evaluated in unlambda, it will not print anything and has the same effect as the starting code.

-- end example]

[Note 1: The above example does not prove anything and just shows a case where it works. I don't know how to prove this. -- end note]

[Example 2:
```
let print = # some side-effect

# This is in normal form according to our reduction strategy and should not print anything.
let main = λx.print # transformed to D (K print)
```
-- end example]

Formula: \
`T (λx.F) = K (T F)` if `F` doesn't mention `x` and `F` is pure \
`T (λx.F) = D (K (T F))` if `F` doesn't mention `x`

### Application in abstraction

For an abstraction `A` in the form `λx.F G`:
- If `F` doesn't mention `x` and `G` is `x`:
    - If `F` is pure, `A` will be transformed to `T F`.
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
`T (λx.F x) = T F` if `F` doesn't mention `x` and `F` is pure \
`T (λx.F x) = D (T F)` if `F` doesn't mention `x` and `F` is unpure \
`T (λx.F G) = S (T (λx.F)) (T (λx.G))`\

### Nested abstraction

An abstraction in the form `λx.λy.F` shall be transformed to: `T (λx.T (λy.F))`

Formula: \
`T (λx.λy.F) = T (λx.T (λy.F))`

### Application

An application in the form `F G` shall be transformed to: `(T F) (T G)`.

Formula: \
`T (F G) = (T F) (T G)`

### Combinator or variable

An expression `F` shall be its own transformation if it is a combinator or a variable.

Formula: \
`T F = F` if `F` is a combinator or a variable

## Required pure expressions

The following expressions are always considered pure:
- variables,
- combinators,
- applications whose left-hand side is `D`.
<!-- - expressions inside pure definitions. -->

[Note 1:

Variables are pure because the to-be-substituted expression will already be delayed elsewhere.

[Example 1:
```
let print = # some side-effect
let main = (λx.x x) print # D will be applied to print in the preprocessing stage, so there is no need to also apply D to the right-hand side x.
```
-- end example]

-- end note]

## Substituting variables

WARNING: This section needs more work. What if we want the expression to be evaluated? Reflect on "The pure specifier".

Any leftover variables after applying transformations will name a definition. They are substituted with the expression from the corresponding definition. If that expression is unpure, `D` is applied to it before substituting.

[Example 1:
```
let print = "a" (λx.x)
let main = print # main is D ("a" (λx.x))
```
-- end example]

## The pure specifier

A definition can be marked pure by adding the *pure specifier* before its name. A *pure definition* is a definition marked with the pure specifier. The expression inside a pure definition is considered pure.

[Note 1: Substituting variables which name pure definitions will never introduce `D`. -- end note]

[Example 1:
```
let pure my_i = λx.x # transforms to I
let main = my_i my_i # substitutes to I I
```
-- end example]

WARNING: I need to think more about this. How does this affect the subexpressions inside the top-level expression?
Think about this:
```
let pure print_hi = "hi" (λx.x)
let main = print_hi
```
And this:
```
let pure K_loop = λ_.(λx.x x) (λx.x x) # should also all the subexpressions be treated as pure?
let main = K_loop
```
