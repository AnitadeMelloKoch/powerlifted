(define (domain minecraft)
    (:requirements :strips)

    (:predicates
        (hungry ?ag)
        (has-logs ?ag)
        (has-planks ?ag)
        (has-sticks ?ag)
        (has-stone ?ag)
        (has-iron ?ag)
        (has-wool ?ag)
        (has-wood-pickaxe ?ag)
        (has-stone-pickaxe ?ag)
        (has-stone-axe ?ag)
        (has-shears ?ag)
        (has-string ?ag)
        (has-honeycomb ?ag)
        (has-candle ?ag)
        (has-bed ?ag)
        (tribe-has-food)
        (cake-ready)
        (candle-lit)
    )

    (:functions
        (total-cost) - number
    )

    ; ---------- resources ----------

    (:action chop-tree
        :parameters (?ag)
        :precondition ()
        :effect (and (has-logs ?ag))
    )

    (:action destroy-bush
        :parameters (?ag)
        :precondition ()
        :effect (and (has-sticks ?ag))
    )

    (:action mine-stone
        :parameters (?ag)
        :precondition (or
            (has-wood-pickaxe ?ag)
            (has-stone-pickaxe ?ag)
        )
        :effect (and (has-stone ?ag))
    )

    (:action mine-iron
        :parameters (?ag)
        :precondition (and (has-stone-pickaxe ?ag))
        :effect (and (has-iron ?ag))
    )

    (:action shear-sheep
        :parameters (?ag)
        :precondition (and (has-shears ?ag))
        :effect (and (has-wool ?ag))
    )

    (:action shear-beehive
        :parameters (?ag)
        :precondition (and (has-shears ?ag))
        :effect (and (has-honeycomb ?ag))
    )

    ; ---------- crafting ----------

    (:action craft-planks
        :parameters (?ag)
        :precondition (and (has-logs ?ag))
        :effect (and
            (not (has-logs ?ag))
            (has-planks ?ag)
        )
    )

    (:action craft-wood-pickaxe
        :parameters (?ag1 ?ag2 ?ag3)
        :precondition (and
            (has-sticks ?ag1)
            (has-planks ?ag2)
        )
        :effect (and
            (not (has-sticks ?ag1))
            (not (has-planks ?ag2))
            (has-wood-pickaxe ?ag3)
        )
    )

    (:action craft-stone-pickaxe
        :parameters (?ag1 ?ag2 ?ag3)
        :precondition (and
            (has-sticks ?ag1)
            (has-stone ?ag2)
        )
        :effect (and
            (not (has-sticks ?ag1))
            (not (has-stone ?ag2))
            (has-stone-pickaxe ?ag3)
        )
    )

    (:action craft-stone-axe
        :parameters (?ag1 ?ag2 ?ag3)
        :precondition (and
            (has-sticks ?ag1)
            (has-stone ?ag2)
        )
        :effect (and
            (not (has-sticks ?ag1))
            (not (has-stone ?ag2))
            (has-stone-axe ?ag3)
        )
    )

    (:action craft-shears
        :parameters (?ag)
        :precondition (and
            (has-iron ?ag)
        )
        :effect (and
            (not (has-iron ?ag))
            (has-shears ?ag)
        )
    )

    (:action craft-bed
        :parameters (?ag)
        :precondition (and
            (has-wool ?ag)
            (has-planks ?ag)
        )
        :effect (and
            (not (has-wool ?ag))
            (not (has-planks ?ag))
            (has-bed ?ag)
        )
    )

    (:action craft-string
        :parameters (?ag)
        :precondition (and
            (has-wool ?ag)
        )
        :effect (and
            (not (has-wool ?ag))
            (has-string ?ag)
        )
    )

    (:action craft-candle
        :parameters (?ag1 ?ag2 ?ag3)
        :precondition (and
            (has-string ?ag1)
            (has-honeycomb ?ag2)
        )
        :effect (and
            (not (has-string ?ag1))
            (not (has-honeycomb ?ag2))
            (has-candle ?ag3)
        )
    )

    ; ---------- food ----------

    (:action hunt
        :parameters (?ag)
        :precondition ()
        :effect (and
            (tribe-has-food)
            (hungry ?ag)
            (increase (total-cost) 50)
        )
    )

    (:action gather
        :parameters (?ag)
        :precondition (and (hungry ?ag))
        :effect (and
            (tribe-has-food)
            (increase (total-cost) 50)
        )
    )

    (:action craft-cake
        :parameters (?ag)
        :precondition (and
            (tribe-has-food)
        )
        :effect (and
            (not (tribe-has-food))
            (cake-ready)
            (increase (total-cost) 20)
        )
    )

    (:action place-candle
        :parameters (?ag)
        :precondition (and
            (has-candle ?ag)
            (cake-ready)
        )
        :effect (and
            (not (has-candle ?ag))
            (candle-lit)
            (increase (total-cost) 1)
        )
    )

    (:action eat-cake
        :parameters (?ag)
        :precondition (and
            (cake-ready)
            (hungry ?ag)
        )
        :effect (and
            (not (cake-ready))
            (not (hungry ?ag))
        )
    )

    (:action eat-candle-cake
        :parameters (?ag)
        :precondition (and
            (cake-ready)
            (candle-lit)
            (hungry ?ag)
        )
        :effect (and
            (not (cake-ready))
            (not (candle-lit))
            (not (hungry ?ag))
        )
    )

    (:action wait
        :parameters (?ag)
        :precondition ()
        :effect (and (hungry ?ag))
    )

)