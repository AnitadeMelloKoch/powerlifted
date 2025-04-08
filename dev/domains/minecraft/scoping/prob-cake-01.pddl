(define (problem minecraft-cake-01)

    (:domain minecraft)

    (:objects
        steve01
    )

    (:init
        (hungry steve01)
        (= (total-cost) 0)
    )

    (:goal (and
        (cake-ready)
        (tribe-has-food)
    ))

    (:metric minimize
        (total-cost)
    )
)