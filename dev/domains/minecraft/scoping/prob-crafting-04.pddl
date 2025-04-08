(define (problem minecraft-crafting-04)

    (:domain minecraft)

    (:objects
        steve01 steve02 steve03 steve04
    )

    (:init
        (hungry steve01)
        (hungry steve02)
        (hungry steve03)
        (hungry steve04)
    )

    (:goal
        (and
            (hungry steve01)
            (hungry steve02)
            (hungry steve03)
            (hungry steve04)
            (has-wood-pickaxe steve01)
            (has-stone-axe steve02)
            (has-stone-pickaxe steve03)
            (has-bed steve04)
        )
    )

)