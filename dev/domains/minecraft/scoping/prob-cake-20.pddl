(define (problem minecraft-cake-20)

    (:domain minecraft)

    (:objects
        steve01
        steve02
        steve03
        steve04
        steve05
        steve06
        steve07
        steve08
        steve09
        steve10
        steve11
        steve12
        steve13
        steve14
        steve15
        steve16
        steve17
        steve18
        steve19
        steve20
    )

    (:init
        (hungry steve01)
        (hungry steve02)
        (hungry steve03)
        (hungry steve04)
        (hungry steve05)
        (hungry steve06)
        (hungry steve07)
        (hungry steve08)
        (hungry steve09)
        (hungry steve10)
        (hungry steve11)
        (hungry steve12)
        (hungry steve13)
        (hungry steve14)
        (hungry steve15)
        (hungry steve16)
        (hungry steve17)
        (hungry steve18)
        (hungry steve19)
        (hungry steve20)
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