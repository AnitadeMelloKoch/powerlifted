(define (problem minecraft-wood-1)

    (:domain minecraft-blocks)

    (:objects
        agent01 - agent
        wood01 - wood
    )

    (:init
        (wood_not_collected wood01)
        (alive agent01)
    )

    (:goal (and
        (has_wood agent01)
    ))

)