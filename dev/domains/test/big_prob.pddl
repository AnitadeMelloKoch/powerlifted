(define (problem Test)
(:domain TEST)
(:objects obj1 obj2 obj3 obj4 l1 l2 l3 l4 l5 l6 agent)
(:INIT (at obj1 l1) (at obj2 l1) (at obj3 l3) (at obj4 l2) (path l1 l2) (path l2 l3) (path l1 l3) (path l3 l4) (path l4 l5) (path l5 l6) (path l6 l4) (path l6 l3) (alive agent))
(:goal (AND (at obj1 l4) (at obj2 l3) (at obj4 l4)))
)