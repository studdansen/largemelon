# Getting End-Location of Text Span at Start-Location

Let $P$ be the location span of the previous text span, and let $T$ be
the matched text for which the location span $Q$ is being found.

```math
Q = \left\{ q_{fl}, q_{fc}, q_{ll}, q_{lc} \right\}
```

The new location starts just after the last position in $P$:

```math
q_{fl} &= p_{ll}
```

```math
q_{fc} &= p_{lc}
```

The number of lines spanned by the new location is equal to the number of
newline character sequences ("newlines") in $T$.

```math
N_{nl} = \mbox{number of newlines in } T
```

```math
t = \mbox{index of first character in } T \mbox{ after last newline}
```

If any newlines were encountered in $T$, then the column number of the last
position spanned by the new location is equal to the number of positions from
the last newline in $T$ to the end of $T$.

If no newlines were encountered, then the column number of the new location is
simply the value of the previous location's last position plus the number of
positions in $T$.

```math
q_{ll} = q_{fl} + N_{nl}
```

```math
q_{lc} = \left\{ \begin{array}{ccl}
  \mbox{len}\left(T\right) - t & \mbox{if} & N_{nl} > 0 \\
  \mbox{len}\left(T\right) + p_{lc} & \mbox{if} & N_{nl} = 0
  \end{array} \right.
```
