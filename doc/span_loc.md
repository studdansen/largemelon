# Building Location of Text Span from Start- and End-Locations

Let $P$ and $Q$ be the respective locations of the first and last character(s)
in a text span such that $P=\left\{p_{fl}, p_{fc}, p_{ll}, p_{lc}\right\}$ and
$Q=\left\{q_{fl}, q_{fc}, q_{ll}, q_{lc}\right\}$. Then:

```math
\begin{array}{lcl}
r_{0,l} & = & \mbox{line number at beginning of text span} \\
  & = & \mbox{min}\left( p_{fl}, q_{fl} \right) \\
r_{1,l} & = & \mbox{line number at end of text span} \\
  & = & \mbox{max}\left( p_{ll}, q_{ll} \right) \\
r_{0,c} & = & \mbox{column number at beginning of text span} \\
  & = & \left\{ \begin{array}{ccl}
    \mbox{min}\left( p_{fc}, q_{fc} \right) & \mbox{if} & p_{fl} = q_{fl} \\
    p_{fc} & \mbox{if} & p_{fl} < q_{fl} \\
    q_{fc} & \mbox{if} & p_{fl} > q_{fl} \\
  \end{array} \right. \\
r_{1,c} & = & \mbox{column number at end of text span} \\
  & = & \left\{ \begin{array}{ccl}
    \mbox{max}\left( p_{lc}, q_{lc} \right) & \mbox{if} & p_{ll} = q_{ll} \\
    q_{lc} & \mbox{if} & p_{ll} < q_{ll} \\
    p_{lc} & \mbox{if} & p_{ll} > q_{ll} \\
  \end{array} \right. \\
\end{array}
```
