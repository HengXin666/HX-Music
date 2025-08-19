from src.mark.AssMark import AssMark

assLine = r"{\kf100}よおこそ実力至上教室へ{\kf20}is {\kf20}good {\kf16}ラ{\kf13}ス{\kf15}ト{\kf11}チ{\kf11}ャ{\kf11}ン{\kf20}ス{\kf20}に{\kf17}飢{\kf20}え{\kf20}た{\kf22}つ{\kf20}ま{\kf39}先{\kf21}が{\kf21}繰{\kf21}り{\kf21}返{\kf21}す{\kf20}学{\kf20}校{\kf20}生{\kf20}活{\kf20}行{\kf20}か{\kf20}な{\kf20}け{\kf20}れ{\kf20}ば{\kf18}This {\kf23}lie {\kf32}is {\kf45}love"

s = AssMark.mark(assLine)

print(s)

s = AssMark.mark(r"{\kf88}微{\kf15}笑{\kf15}ん{\kf14}で{\kf14}き{\kf14}っ{\kf64}と{\kf41}固{\kf20}い{\kf46}縁{\kf55}が{\k22}繋{\k14}ぐ")

# {\k119}{\k64}微笑|<ほ{\k29}#|<ほ{\k8}#|<え{\k11}ん{\k12}で{\k17}き{\k22}っ{\k64}と{\k22}固|<か{\k21}#|<た{\k24}い{\k15}縁|<よ{\k15}#|<す{\k19}#|<が{\k88}が{\k22}繋|<つ{\k22}#|<な{\k14}ぐ
print("\n", s)