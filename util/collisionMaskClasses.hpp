#ifndef COLLISIONMASCLASSES_H
#define COLLISIONMASCLASSES_H
/* collision filtering masks.
 * These controll what types of objects can collide
 * with others in the world. Keep in mind that "collision
 * objects" in this regard includes caycasts and
 * proximity fields.
 *
 * ==>> What little documentation I could find on this: <<==
 * https://web.archive.org/web/20180303210810/http://www.bulletphysics.org/mediawiki-1.5.8/index.php/Collision_Filtering
 * https://pybullet.org/Bullet/phpBB3/viewtopic.php?t=12154
 * https://pybullet.org/Bullet/phpBB3/viewtopic.php?t=7538
 * https://pybullet.org/Bullet/phpBB3/viewtopic.php?t=8752
 */


#define BIT(x) (1<<(x))
//collision classes: each physics body must belong to one of these classes
enum collisionMasks { //groups
    COL_NOTHING = 0, //<Collide with nothing
    COL_TERRAIN = BIT(2), //<Collide with terrain
    COL_SELECTER = BIT(3), //<Collide with raycasts used to select objects with cursor
    COL_DAMAGEABLE = BIT(4), //Collidable with damage causing objects
    COL_POWERUP = BIT(5), //<Collide with powerups
    COL_PLAYER = BIT(6),
    COL_THROWABLE = BIT(7),
    COL_ITEM = BIT(8),
    COL_PROXIMITY = BIT(9),
};

// What classes an object collides with. These can be different for each object
// Note: for 2 objects to collide, they MUST both be set to collide with each other.
// If only one is set to collide, collision will not happen.
enum collisionGroups { //masks
    COL_TERRAIN_COLLIDES_WITH =     COL_TERRAIN | COL_SELECTER | COL_POWERUP | COL_PLAYER | COL_THROWABLE | COL_ITEM                 ,
    COL_SELECT_RAY_COLLIDES_WITH =  COL_TERRAIN |                COL_POWERUP | COL_PLAYER | COL_THROWABLE | COL_ITEM                 ,
    COL_POWERUP_COLLIDES_WITH =     COL_TERRAIN | COL_SELECTER |               COL_PLAYER                                            ,
    COL_PLAYER_COLLIDES_WITH =      COL_TERRAIN | COL_SELECTER | COL_POWERUP | COL_PLAYER | COL_THROWABLE | COL_ITEM | COL_PROXIMITY ,
    COL_THROWABLE_COLLIDES_WITH =   COL_TERRAIN | COL_SELECTER |               COL_PLAYER | COL_THROWABLE |            COL_PROXIMITY ,
    COL_ITEM_COLLIDES_WITH =        COL_TERRAIN | COL_SELECTER |               COL_PLAYER                                            ,
    COL_PROXIMITY_COLLIDES_WITH =                                              COL_PLAYER | COL_THROWABLE                            ,


    COL_NOTHING_COLLIDES_WITH = 0
};

#endif
