(define (domain Test)
    (:predicates (at ?x ?y)
                 (path ?l1 ?l2)
                 (alive ?a))

    (:action a1
        :parameters (?x ?y ?w ?z ?ag)
        :precondition (and (at ?x ?y) 
                           (path ?y ?w)
                           (path ?w ?z)
                           (alive ?ag)
                           )
        :effect
        (and (at ?x ?z)
             (not (at ?x ?y))))
    
    (:action a2
        :parameters (?x ?y ?w ?z ?ag)
        :precondition (and (at ?x ?z)
                            (path ?y ?w)
                            (path ?w ?z)
                            (alive ?ag)
                            )
        :effect
        (and (at ?x ?y)
             (not (at ?x ?z))))

)