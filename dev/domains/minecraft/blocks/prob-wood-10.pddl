(define (problem minecraft-wood-10)

    (:domain minecraft-blocks)

    (:objects
        agent01 - agent
        wood01 wood02 wood03 wood04 wood05 wood06 wood07 wood08 wood09 wood10 - wood
        stone01 stone02 stone03 stone04 stone05 stone06 stone07 stone08 stone09  stone10 - stone
    )

    (:init
        (wood_not_collected wood01)
        (wood_not_collected wood02)
        (wood_not_collected wood03)
        (wood_not_collected wood04)
        (wood_not_collected wood05)
        (wood_not_collected wood06)
        (wood_not_collected wood07)
        (wood_not_collected wood08)
        (wood_not_collected wood09)
        (wood_not_collected wood10)
        (stone_not_collected stone01)
        (stone_not_collected stone02)
        (stone_not_collected stone03)
        (stone_not_collected stone04)
        (stone_not_collected stone05)
        (stone_not_collected stone06)
        (stone_not_collected stone07)
        (stone_not_collected stone08)
        (stone_not_collected stone09)
        (stone_not_collected stone10)
    )

    (:goal (and
        (has_wood agent01)
    ))

)