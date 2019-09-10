// weapon.cpp: all shooting and effects code, projectile management
#include "game.h"

extern int zoom;

namespace game
{
    VAR( hasweapons, 0, 1, 1 );
    VAR( lastzoom, -1, 0, 1 );

    static const int OFFSETMILLIS = 500;
    vec rays[MAXRAYS];

    struct hitmsg
    {
        int target
        ,lifesequence
        ,info1
        ,info2
        ,flags
        ;
        ivec dir;
    };
    vector<hitmsg> hits;

#if 0
    #define MINDEBRIS 3
    VARP(maxdebris, MINDEBRIS, 10, 100);
    VARP(maxgibs, 0, 4, 100);
#endif

    void offsetray(const vec &from, const vec &to, int spread, float range, vec &dest)
    {
        vec offset;
        do offset = vec(rndscale(1), rndscale(1), rndscale(1)).sub(0.5f);
        while(offset.squaredlen() > 0.5f*0.5f);
        offset.mul((to.dist(from)/1024)*spread);
        offset.z /= 2;
        dest = vec(offset).add(to);
        if(dest != from)
        {
            vec dir = vec(dest).sub(from).normalize();
            raycubepos(from, dir, dest, range, RAY_CLIPMAT|RAY_ALPHAPOLY);
        }
    }

    void createrays(int atk, const vec &from, const vec &to)             // create random spread of rays
    {
        if( atk == NO_SKILL ) {
            return;
        }
        loopi(skills[atk].rays) offsetray(from, to, skills[atk].spread, skills[atk].range, rays[i]);
    }

    void hit(int damage, dynent *d, gameent *at, const vec &vel, int atk, float info1, int info2 = 1, int flags = 0)
    {
        if(at==player1 && d!=at)
        {
            if( lasthit != lastmillis ) playsound(S_HIT);
            lasthit = lastmillis;
        }

        // f = victim
        gameent *f = (gameent *)d;

        if(at->type==ENT_PLAYER && !isteam(at->team, f->team)) {
            at->totaldamage += damage;
        }

        if( f==at ) f->hitpush(damage, vel, at, atk);

        hitmsg &h = hits.add();
        h.target = f->clientnum;
        h.lifesequence = f->lifesequence;
        h.info1 = int(info1*DMF);
        h.info2 = info2;
        h.flags = flags;
        h.dir = f==at ? ivec(0, 0, 0) : ivec(vec(vel).mul(DNF));
    }

    void hitpush(int damage, dynent *d, gameent *at, vec &from, vec &to, int atk, int rays)
    {
        int flags = 0;
        if( d->type == ENT_PLAYER ) {
            gameent *ged = (gameent*)d;
            if( to.dist( ged->taghead ) < HEADSHOT_DIST ) {
                flags = FL_HEADSHOT;
            }
        }

        hit(damage, d, at, vec(to).sub(from).safenormalize(), atk, from.dist(to), rays, flags);
    }

    struct bouncer : physent
    {
        int lifetime, bounces;
        float lastyaw, roll;
        bool local;
        gameent *owner;
        int atk;
        int bouncetype, variant;
        vec offset;
        int offsetmillis;
        int id;
        vector< int > touched;

        bouncer() : bounces(0), roll(0), variant(0)
        {
            type = ENT_BOUNCE;
        }
    };

    vector<bouncer *> bouncers;

    void newbouncer(const vec &from, const vec &to, bool local, int id, gameent *owner, int type, int lifetime, int speed, int atk)
    {
        bouncer &bnc = *bouncers.add(new bouncer);
        bnc.o = from;
        bnc.radius = bnc.xradius = bnc.yradius = type==BNC_DEBRIS ? 0.5f : 1.5f;
        bnc.eyeheight = bnc.radius;
        bnc.aboveeye = bnc.radius;
        bnc.lifetime = lifetime;
        bnc.local = local;
        bnc.owner = owner;
        bnc.bouncetype = type;
        bnc.id = local ? lastmillis : id;
        bnc.atk = atk;

        switch(type)
        {
            case BNC_DEBRIS: bnc.variant = rnd(4); break;
            case BNC_GIBS: bnc.variant = rnd(3); break;
        }

        vec dir(to);
        dir.sub(from).safenormalize();
        bnc.vel = dir;
        bnc.vel.mul(speed);

        avoidcollision(&bnc, dir, owner, 0.1f);

        bnc.offset = from;
        bnc.offset.sub(bnc.o);
        bnc.offsetmillis = OFFSETMILLIS;

        bnc.resetinterp();
    }

    void bounced(physent *d, const vec &surface)
    {
        if(d->type != ENT_BOUNCE) return;
        bouncer *b = (bouncer *)d;
        if(b->bouncetype != BNC_GIBS || b->bounces >= 2) return;
        b->bounces++;
        addstain(STAIN_BLOOD, vec(b->o).sub(vec(surface).mul(b->radius)), surface, 2.96f/b->bounces, bvec(0x60, 0xFF, 0xFF), rnd(4));
    }

    void updatebouncers(int time)
    {
        loopv(bouncers)
        {
            bouncer &bnc = *bouncers[i];
            bool isweapon = bnc.bouncetype == BNC_GRENADE;
            vec old(bnc.o);
            bool stopped = false;
            bool exploded = false;
            bool isowner = bnc.owner == player1;
            // cheaper variable rate physics for debris, gibs, etc.
            for(int rtime = time; rtime > 0;)
            {
                int qtime = min(30, rtime);
                rtime -= qtime;
                if((bnc.lifetime -= qtime)<0 || bounce(&bnc, qtime/1000.0f, 0.4f, 0.5f, 1)) { stopped = true; break; }
            }

            bnc.roll += old.sub(bnc.o).magnitude()/(4*RAD);
            bnc.offsetmillis = max(bnc.offsetmillis-time, 0);

            if( isweapon ) {
                if( stopped ) {
                    exploded = true;
                }
            }

            if( bnc.local )
            {
                hits.setsize(0);
                loopj(numdynents())
                {
                    dynent *o = iterdynents(j);
                    bool canbetouched = true;
                    loopk( bnc.touched.length() ) {
                        int id = bnc.touched[k];
                        if( id == j ) {
                            canbetouched = false;
                            break;
                        }
                    }
                    if( !canbetouched ) {
                        continue;
                    }
                    if(o->o.reject(bnc.o, skills[bnc.atk].exprad)) continue;
                    bnc.touched.add( j );
                    hit(skills[bnc.atk].damage, o, bnc.owner, bnc.vel, bnc.atk, 0);
                }
                if( hits.length() ) {
                    vec from = bnc.o;
                    vec to = bnc.o;
                    addmsg(N_USE_SKILL, "rci2i6iv", bnc.owner, lastmillis-maptime, bnc.atk,
                           (int)(from.x*DMF), (int)(from.y*DMF), (int)(from.z*DMF),
                           (int)(to.x*DMF), (int)(to.y*DMF), (int)(to.z*DMF),
                           hits.length(), hits.length()*sizeof(hitmsg)/sizeof(int), hits.getbuf());
                }
            }

            if(stopped)
            {
                delete bouncers.remove(i--);
            }
        }
    }

    void removebouncers(gameent *owner)
    {
        loopv(bouncers) if(bouncers[i]->owner==owner) { delete bouncers[i]; bouncers.remove(i--); }
    }

    void clearbouncers() { bouncers.deletecontents(); }

    struct projectile
    {
        vec dir, o, from, to, offset;
        float speed;
        gameent *owner;
        int atk;
        bool local;
        int offsetmillis;
        int id;
        int color;
    };
    vector<projectile> projs;

    void clearprojectiles() { projs.shrink(0); }

    void newprojectile(const vec &from, const vec &to, float speed, bool local, int id, gameent *owner, int atk, int color)
    {
        projectile &p = projs.add();
        p.dir = vec(to).sub(from).safenormalize();
        p.o = from;
        p.from = from;
        p.to = to;
        p.offset = vec();
        p.offset.sub(from);
        p.speed = speed;
        p.local = local;
        p.owner = owner;
        p.atk = atk;
        p.offsetmillis = OFFSETMILLIS;
        p.id = local ? lastmillis : id;
        p.color = color;
    }

    void removeprojectiles(gameent *owner)
    {
        // can't use loopv here due to strange GCC optimizer bug
        int len = projs.length();
        loopi(len) if(projs[i].owner==owner) { projs.remove(i--); len--; }
    }

    VARP(blood, 0, 1, 1);

    void damageeffect(int damage, gameent *d, bool thirdperson)
    {
        vec p = d->o;
        p.z += 0.6f*(d->eyeheight + d->aboveeye) - d->eyeheight;
        if(blood) particle_splash(PART_BLOOD, max(damage/10, rnd(3)+1), 1000, p, 2.96f);
#if 0
        if(thirdperson) particle_textcopy(d->abovehead(), tempformatstring("%d", damage), PART_TEXT, 2000, 0xFF4B19, 4.0f, -8);
#endif
    }

    void spawnbouncer(const vec &p, const vec &vel, gameent *d, int type)
    {
        vec to(rnd(100)-50, rnd(100)-50, rnd(100)-50);
        if(to.iszero()) to.z += 1;
        to.normalize();
        to.add(p);
        newbouncer(p, to, true, 0, d, type, rnd(1000)+1000, rnd(100)+20, 0);
    }

    void gibeffect(int damage, const vec &vel, gameent *d)
    {
#if 0
        if(!blood || !maxgibs || damage < 0) return;
        vec from = d->abovehead();
        loopi(rnd(maxgibs)+1) spawnbouncer(from, vel, d, BNC_GIBS);
#endif
    }

    float projdist(dynent *o, vec &dir, const vec &v, const vec &vel)
    {
        vec middle = o->o;
        middle.z += (o->aboveeye-o->eyeheight)/2;
        dir = vec(middle).sub(v).add(vec(vel).mul(5)).safenormalize();

        float low = min(o->o.z - o->eyeheight + o->radius, middle.z),
              high = max(o->o.z + o->aboveeye - o->radius, middle.z);
        vec closest(o->o.x, o->o.y, clamp(v.z, low, high));
        return max(closest.dist(v) - o->radius, 0.0f);
    }

    void radialeffect(dynent *o, const vec &v, const vec &vel, int qdam, gameent *at, int atk)
    {
        if(o->state!=CS_ALIVE) return;
        vec dir;
        float dist = projdist(o, dir, v, vel);
        if(dist<skills[atk].exprad)
        {
            float damage = qdam*(1-dist/EXP_DISTSCALE/skills[atk].exprad);
            if(o==at) damage /= EXP_SELFDAMDIV;
            if(damage > 0) hit(max(int(damage), 1), o, at, dir, atk, dist);
        }
    }

    void explode(bool local, gameent *owner, const vec &v, const vec &vel, dynent *safe, int damage, int atk)
    {
        int color = game::particlecolor( PART_SPARK );

        particle_splash(PART_SPARK, 200, 300, v, 0.45f);
        playsound(S_PULSEEXPLODE, &v);
        particle_fireball(v, 1.15f*skills[atk].exprad, PART_PULSE_BURST, int(skills[atk].exprad*20), color, 4.0f);
        vec debrisorigin = vec(v).sub(vec(vel).mul(5));
        
        vec vcol;
        vcol.x = (float)( (color >> 16) & 0xFF ) / 255.0f;
        vcol.y = (float)( (color >> 8) & 0xFF ) / 255.0f;
        vcol.z = (float)( (color >> 0) & 0xFF ) / 255.0f;

        adddynlight(safe ? v : debrisorigin, 2*skills[atk].exprad, vcol, 350, 40, 0, skills[atk].exprad/2, vec(0.5f, 1.5f, 2.0f));
#if 1
        int numdebris = 100;//maxdebris > MINDEBRIS ? rnd(maxdebris-MINDEBRIS)+MINDEBRIS : min(maxdebris, MINDEBRIS);
        if(numdebris)
        {
            vec debrisvel = vec(vel).neg();
            loopi(numdebris)
                spawnbouncer(debrisorigin, debrisvel, owner, BNC_DEBRIS);
        }
#endif

        if(!local) return;
        int numdyn = numdynents();
        loopi(numdyn)
        {
            dynent *o = iterdynents(i);
            if(o->o.reject(v, o->radius + skills[atk].exprad) || o==safe) continue;
            radialeffect(o, v, vel, damage, owner, atk);
        }
    }

    void pulsestain(const projectile &p, const vec &pos)
    {
        vec dir = vec(p.dir).neg();
        int color = game::particlecolor( PART_SPARK );
        float rad = skills[p.atk].exprad*0.75f;
        addstain(STAIN_PULSE_SCORCH, pos, dir, rad);
        addstain(STAIN_PULSE_GLOW, pos, dir, rad, color);
    }

    void projsplash(projectile &p, const vec &v, dynent *safe)
    {
        explode(p.local, p.owner, v, p.dir, safe, skills[p.atk].damage, p.atk);
        pulsestain(p, v);
    }

    void explodeeffects(int atk, gameent *d, bool local, int id)
    {
        if(local) return;
        switch(atk)
        {
            //case ATK_PULSE_SHOOT:
            //case ATK_RL_SHOOT:
            //case ATK_GL_SHOOT:
            //    loopv(projs)
            //    {
            //        projectile &p = projs[i];
            //        if(p.atk == atk && p.owner == d && p.id == id && !p.local)
            //        {
            //            vec pos = vec(p.offset).mul(p.offsetmillis/float(OFFSETMILLIS)).add(p.o);
            //            explode(p.local, p.owner, pos, p.dir, NULL, 0, atk);
            //            pulsestain(p, pos);
            //            projs.remove(i);
            //            break;
            //        }
            //    }
            //    break;
            default:
                break;
        }
    }

    bool projdamage(dynent *o, projectile &p, const vec &v)
    {
        if(o->state!=CS_ALIVE) return false;
        if(!intersect(o, p.o, v, skills[p.atk].margin)) return false;
        projsplash(p, v, o);
        vec dir;
        projdist(o, dir, v, p.dir);
        hit(skills[p.atk].damage, o, p.owner, dir, p.atk, 0);
        return true;
    }

    void updateprojectiles(int time)
    {
        if(projs.empty()) return;
        gameent *noside = hudplayer();
        loopv(projs)
        {
            projectile &p = projs[i];
            int color = p.color;
            p.offsetmillis = max(p.offsetmillis-time, 0);
            vec dv;
            float dist = p.to.dist(p.o, dv);
            dv.mul(time/max(dist*1000/p.speed, float(time)));
            vec v = vec(p.o).add(dv);
            bool exploded = false;
            hits.setsize(0);
            if(p.local)
            {
                vec halfdv = vec(dv).mul(0.5f), bo = vec(p.o).add(halfdv);
                float br = max(fabs(halfdv.x), fabs(halfdv.y)) + 1 + skills[p.atk].margin;
                loopj(numdynents())
                {
                    dynent *o = iterdynents(j);
                    if(p.owner==o || o->o.reject(bo, o->radius + br)) continue;
                    if(projdamage(o, p, v)) { exploded = true; break; }
                }
            }
            if(!exploded)
            {
                if(dist<4)
                {
                    if(p.o!=p.to) // if original target was moving, reevaluate endpoint
                    {
                        if(raycubepos(p.o, p.dir, p.to, 0, RAY_CLIPMAT|RAY_ALPHAPOLY)>=4) continue;
                    }
                    projsplash(p, v, NULL);
                    exploded = true;
                }
                else
                {
                    vec pos = vec(p.offset).mul(p.offsetmillis/float(OFFSETMILLIS)).add(v);
                    particle_splash(PART_PULSE_FRONT, 1, 1, pos, 2.4f, 150, 20);
                    if(p.owner != noside)
                    {
                        float len = min(20.0f, vec(p.offset).add(p.from).dist(pos));
                        vec dir = vec(dv).normalize(),
                            tail = vec(dir).mul(-len).add(pos),
                            head = vec(dir).mul(2.4f).add(pos);
                        particle_flare(tail, head, 1, PART_PULSE_SIDE, color, 2.5f);
                    }
                }
            }
            if(exploded)
            {
                projs.remove(i--);
            }
            else p.o = v;
        }
    }

    void bullethit(const vec &from, const vec &to, int size=1, bool stain = true)
    {
        particle_flare( from, to, 250, PART_RAIL_TRAIL, game::particlecolor( PART_SPARK ), 0.2f * size);
        particle_splash(PART_STEAM, 50, 100, to, 0.65f, 150, 1 );
        particle_splash(PART_SPARK, 6, 200, to, 0.45f);
        vec dir = vec(from).sub(to).safenormalize();
        if(stain)
        {
            addstain(STAIN_RAIL_HOLE, to, dir, 2.0f);
            addstain(STAIN_RAIL_GLOW, to, dir, 2.5f, game::particlecolor( PART_SPARK ));
        }
        vec vcol( 1.0f, 1.0f, 1.0f );
        adddynlight(vec(to).madd(dir, 4), 2, vcol, 225, 75);
    }

    void shoteffects( int atk, const vec &from, const vec &to, gameent *d, bool local, int id, int prevaction, int flags )     // create visual effect from a shot
    {
        int color = game::particlecolor( PART_SPARK );
        int hudcolor = color;

        if( atk != NO_SKILL ) {
            const skillinfo& s = skills[ atk ];
            if( ( s.fx & FL_FX_MUZZLE ) ) {
                particle_flare(d->muzzle, d->muzzle, 140, PART_SMOKE, 0x303020, 3.50f, d);
            }

            if( ( s.fx & FL_FX_POWDER ) ) {
                particle_splash(PART_SPARK, 10, 50, d->powder, 0.45f);
                vec vcol( 1.0f, 1.0f, 1.0f );
                adddynlight( d->powder, 35, vcol, 75, 75 );
            }

        }

        switch(atk)
        {
            //case ATK_PULSE_SHOOT:
            //    if(d->muzzle.x >= 0)
            //        particle_flare(d->muzzle, d->muzzle, 140, PART_PULSE_MUZZLE_FLASH, hudcolor, 3.50f, d);
            //    newprojectile(from, to, skills[atk].projspeed, local, id, d, atk, color);
            //    break;

            //case ATK_RL_SHOOT:
            //    particle_flare(d->muzzle, d->muzzle, 140, PART_PULSE_MUZZLE_FLASH, hudcolor, 3.50f, d);
            //    newprojectile(from, to, skills[atk].projspeed, local, id, d, atk, color);
            //    break;

            //case ATK_GL_SHOOT:
            //    particle_flare(d->muzzle, d->muzzle, 140, PART_PULSE_MUZZLE_FLASH, hudcolor, 3.50f, d);
            //    newbouncer(from, to, local, id, d, BNC_GRENADE, 1000, skills[atk].projspeed, atk);
            //    break;

            //case ATK_RAIL_SHOOT:
            //    if(d->muzzle.x >= 0)
            //        particle_flare(d->muzzle, d->muzzle, 140, PART_RAIL_MUZZLE_FLASH, hudcolor, 3.15f, d);
            //    adddynlight(hudgunorigin(gun, d->o, to, d), 35, vcol, 75, 75, DL_FLASH, 0, vec(0, 0, 0), d);
            //    break;

            //case ATK_SG_SHOOT:
            //    if(d->muzzle.x >= 0)
            //        particle_flare(d->muzzle, d->muzzle, 140, PART_RAIL_MUZZLE_FLASH, hudcolor, 2.75f, d);
            //    adddynlight(hudgunorigin(gun, d->o, to, d), 35, vcol, 75, 75, DL_FLASH, 0, vec(0, 0, 0), d);
            //    break;

            default:
                break;
        }

        bool localPlayer = d == hudplayer();

        if( ( flags & FL_PARRY ) ) {
            particle_splash(PART_SPARK, 10, 100, d->o, 0.30f);
            vec vcol( 1.0f, 1.0f, 1.0f );
            adddynlight( d->o, 20, vcol, 75, 75 );

            int parrysound = skills[atk].parrysound;
            if( localPlayer ) {
                playsound( parrysound );
            }
            else {
                playsound( parrysound, &d->o );
            }
        }
        else if( ( flags & FL_BLOCKED ) ) {
            particle_splash(PART_SPARK, 10, 100, d->o, 0.30f);
            vec vcol( 1.0f, 1.0f, 1.0f );
            adddynlight( d->o, 20, vcol, 75, 75 );

            int blocksound = skills[atk].blocksound;
            if( localPlayer ) {
                playsound( blocksound );
            }
            else {
                playsound( blocksound, &d->o );
            }
        }

        if( skills[atk].owned ) {
            if( localPlayer ) {
                //playsound( S_ATTACK_START, NULL );
                playsound(skills[atk].hudsound, NULL);
            }
            else {
                //playsound( S_ATTACK_START, &d->o );
                playsound(skills[atk].sound, &d->o);
            }
        }
    }

    void particletrack(physent *owner, vec &o, vec &d)
    {
        if(owner->type!=ENT_PLAYER) return;
        gameent *pl = (gameent *)owner;
        if(pl->muzzle.x < 0 || pl->lastskill == NO_SKILL ) return;
        float dist = o.dist(d);
        o = pl->muzzle;
        if(dist <= 0) d = o;
        else
        {
            vecfromyawpitch(owner->yaw, owner->pitch, 1, 0, d);
            float newdist = raycube(owner->o, d, dist, RAY_CLIPMAT|RAY_ALPHAPOLY);
            d.mul(min(newdist, dist)).add(owner->o);
        }
    }

    void dynlighttrack(physent *owner, vec &o, vec &hud)
    {
        if(owner->type!=ENT_PLAYER) return;
        gameent *pl = (gameent *)owner;
        if(pl->muzzle.x < 0 || pl->lastskill < 0 ) return;
        o = pl->muzzle;
        hud = owner == hudplayer() ? vec(pl->o).add(vec(0, 0, 2)) : pl->muzzle;
    }

    float intersectdist = 1e16f;

    bool intersect(dynent *d, const vec &from, const vec &to, float margin, float &dist)   // if lineseg hits entity bounding box
    {
        vec bottom(d->o), top(d->o);
        bottom.z -= d->eyeheight + margin;
        top.z += d->aboveeye + margin;
        return linecylinderintersect(from, to, bottom, top, d->radius + margin, dist);
    }

    dynent *intersectclosest(const vec &from, const vec &to, gameent *at, float margin, float &bestdist)
    {
        dynent *best = NULL;
        bestdist = 1e16f;
        loopi(numdynents())
        {
            dynent *o = iterdynents(i);
            if(o==at || o->state!=CS_ALIVE) continue;
            float dist;
            if(!intersect(o, from, to, margin, dist)) continue;
            if(dist<bestdist)
            {
                best = o;
                bestdist = dist;
            }
        }
        return best;
    }

    void shorten(const vec &from, vec &target, float dist)
    {
        target.sub(from).mul(min(1.0f, dist)).add(from);
    }

    void raydamage(vec &from, vec &to, gameent *d, int skillid)
    {
        if( skillid == NO_SKILL ) {
            return;
        }
        dynent *o;
        float dist;
        int maxrays = skills[skillid].rays
        , margin = skills[skillid].margin
        , size = skills[skillid].bulletsize
        ;
        if(skills[skillid].rays > 1)
        {
            dynent *dhits[MAXRAYS];
            loopi(maxrays)
            {
                dhits[i] = intersectclosest(from, rays[i], d, margin, dist);
            }
            loopi(maxrays) if(dhits[i])
            {
                o = dhits[i];
                dhits[i] = NULL;
                int numhits = 1;
                for(int j = i+1; j < maxrays; j++) if(dhits[j] == o)
                {
                    dhits[j] = NULL;
                    numhits++;
                }
                hitpush(numhits*skills[skillid].damage, o, d, from, to, skillid, numhits);
            }
        }
        else if((o = intersectclosest(from, to, d, margin, dist)))
        {
            hitpush(skills[skillid].damage, o, d, from, to, skillid, 1);
        }
    }

    void special( gameent* d ) {
        // cloak
        bool hascloak = classes[ d->classid ].hascloak;

        if( hascloak ) {
            int cloakdelay = classes[ d->classid ].cloakdelay;
            int prev = d->cloakmode;
            d->cloakmode = lastmillis - d->lastaction > cloakdelay ? 1 : 0;
            if( d->cloakmode != prev ) {
                addmsg( N_SWITCH_CLOAK_MODE, "ri", d->cloakmode );
            }
        }
    }

    void reload( gameent* d ) {
        if( d->nextskill
        || d->lastfiredatk < 0 ) {
            return;
        }

        int delta = lastmillis - d->reloadtime;
        int skill = d->lastskill;

        if( skill == NO_SKILL
        || delta < skills[skill].reloaddelay ) {
            return;
        }
        
        /*if( d->ammo[gun] < skills[skill].magazine ) {
            if(d == player1) {
                playsound( S_JUMPPAD, NULL, NULL, 0 );
            }
            ++d->ammo[gun];
            addmsg(N_AMMO_ADD, "rii", gun, 1 );
            d->reloadtime = lastmillis;
        }*/
    }

    bool canshoot( gameent *d ) {
        int attacktime = lastmillis-d->gunstarttime;

        if( !hasweapons
        || d->stuntime > lastmillis
        || attacktime<d->gunwait
        || d->grabentity != -1 ) {
            return false;
        }

        return true;
    }

    void shoot( gameent *d, const vec &targ ) {
        int prevaction = d->lastaction,
        atk = d->lastskill;
        bool offground = d->timeinair && !d->inwater;

        if( d == player1 ) {
            if( zoom != lastzoom ) {
                if( zoom == -1 ) {
                    execident( "fxscopeoff" );
                }
                else if( zoom == 1 ) {
                    execident( "fxscopeon" );
                }
                lastzoom = zoom;
            }
            zoom = -1;
        }

        if( !canshoot( d ) ) {
            return;
        }
        
        d->gunwait = 0;
        d->gunfires = 0;

        if( d->nextskill != NO_SKILL ) {
            if( !d->skilltrigger ) {
                d->gunstarttime = 0;
                d->aimstarttime = lastmillis;    
            }
            if( d == player1
            && skills[ d->nextskill ].haszoom ) {
                zoom = 1;
            }
            d->speed = d->walkspeed;
            d->lastskill = d->nextskill;
            d->skilltrigger = true;
            return;
        }

        d->speed = d->normalspeed;

        if( !d->skilltrigger ) {
            return;
        }
        atk = d->lastskill;
        d->lastaction = lastmillis;
        d->skilltrigger = false;
        d->reloadtime = lastmillis;
        d->lastfiredatk = atk;

        d->gunfires = 1;
        
        d->gunstarttime = lastmillis;

        if( !offground )
        if( skills[atk].prekickamount != 0 ) {
            vec from = d->o, to = targ, dir = vec(to).sub(from).safenormalize();
            vec kickback = vec(dir).mul(skills[atk].prekickamount*-2.5f);
            d->vel.add(kickback);
        }

        d->gunwait = skills[atk].attackdelay;
        d->totalshots += skills[atk].damage*skills[atk].rays;

        vec from = d->o, to = targ, dir = vec(to).sub(from).safenormalize();
        float dist = to.dist(from);
        if( !offground )
        if(!(d->physstate >= PHYS_SLOPE && d->crouching && d->crouched()))
        {
            vec kickback = vec(dir).mul(skills[atk].kickamount*-2.5f);
            d->vel.add(kickback);
        }
        float shorten = skills[atk].range && dist > skills[atk].range ? skills[atk].range : 0,
              barrier = raycube(d->o, dir, dist, RAY_CLIPMAT|RAY_ALPHAPOLY);
        if(barrier > 0 && barrier < dist && (!shorten || barrier < shorten))
            shorten = barrier;
        if(shorten) to = vec(dir).mul(shorten).add(from);

        if(skills[atk].rays > 1) createrays(atk, from, to);
        else if(skills[atk].spread) offsetray(from, to, skills[atk].spread, skills[atk].range, to);

        hits.setsize(0);

        if(!skills[atk].projspeed)  {
            raydamage(from, to, d, atk);
        }

        if(d==player1 || d->ai)
        {
            addmsg(N_USE_SKILL, "rci2i6iv", d, lastmillis-maptime, atk,
                   (int)(from.x*DMF), (int)(from.y*DMF), (int)(from.z*DMF),
                   (int)(to.x*DMF), (int)(to.y*DMF), (int)(to.z*DMF),
                   hits.length(), hits.length()*sizeof(hitmsg)/sizeof(int), hits.getbuf());
        }
    }

    void blockeffect( gameent *d ) {
        static const int advangate_for_blocking = - 300;
        int lastskill = d->lastskill;
        if( lastskill == NO_SKILL ) {
            return;
        }
        playsound( skills[ lastskill ].blocksound );
        d->nextskill = NO_SKILL;
        d->lastaction = lastmillis;
        d->skilltrigger = false;
        d->reloadtime = lastmillis;
        d->lastfiredatk = lastskill;

        d->gunfires = 1;
        
        d->gunstarttime = lastmillis + advangate_for_blocking;

        d->gunwait = skills[lastskill].attackdelay + advangate_for_blocking;
    }

    void adddynlights()
    {
        loopv(projs)
        {
            projectile &p = projs[i];
            //if(p.atk!=ATK_PULSE_SHOOT) continue;
            //if(p.atk!=ATK_RL_SHOOT) continue;
            //if(p.atk!=ATK_GL_SHOOT) continue;
            vec pos(p.o);
            pos.add(vec(p.offset).mul(p.offsetmillis/float(OFFSETMILLIS)));
            adddynlight(pos, 20, vec(0.25f, 0.75f, 1.0f));
        }
    }

#if 0
    static const char * const gibnames[3] = { "gibs/gib01", "gibs/gib02", "gibs/gib03" };
    static const char * const debrisnames[4] = { "debris/debris01", "debris/debris02", "debris/debris03", "debris/debris04" };
#endif

    void preloadbouncers()
    {
        preloadmodel( "mapmodel/obj_cube" );
#if 0
        loopi(sizeof(gibnames)/sizeof(gibnames[0])) preloadmodel(gibnames[i]);
        loopi(sizeof(debrisnames)/sizeof(debrisnames[0])) preloadmodel(debrisnames[i]);
#endif
    }

    void renderbouncers()
    {
        float yaw, pitch;
        loopv(bouncers)
        {
            bouncer &bnc = *bouncers[i];
            vec pos(bnc.o);
            pos.add(vec(bnc.offset).mul(bnc.offsetmillis/float(OFFSETMILLIS)));
            vec vel(bnc.vel);
            if(vel.magnitude() <= 25.0f) yaw = bnc.lastyaw;
            else
            {
                vectoyawpitch(vel, yaw, pitch);
                yaw += 90;
                bnc.lastyaw = yaw;
            }
            pitch = -bnc.roll;
            const char *mdl = NULL;
            int cull = MDL_CULL_VFC|MDL_CULL_DIST|MDL_CULL_OCCLUDED;
            float fade = 1;
            if(bnc.lifetime < 250) fade = bnc.lifetime/250.0f;
            switch(bnc.bouncetype)
            {
                case BNC_GRENADE: mdl = models[ MODEL_CANNONBALL ].name; break;
#if 0
                case BNC_GIBS: mdl = gibnames[bnc.variant]; break;
                case BNC_DEBRIS: mdl = debrisnames[bnc.variant]; break;
#endif
                default: continue;
            }
            rendermodel(mdl, ANIM_MAPMODEL|ANIM_LOOP, pos, yaw, pitch, 0, cull, NULL, NULL, 0, 0, fade);
        }
    }

    void renderprojectiles()
    {
    }

    void removeweapons(gameent *d)
    {
        removebouncers(d);
        removeprojectiles(d);
    }

    void updateweapons(int curtime)
    {
        updateprojectiles(curtime);
        if(player1->clientnum>=0 && player1->state==CS_ALIVE) {
            shoot( player1, worldpos ); // only shoot when connected to server
            reload(player1);
            special(player1);
        }
        updatebouncers(curtime); // need to do this after the player shoots so bouncers don't end up inside player's BB next frame
    }

    void avoidweapons(ai::avoidset &obstacles, float radius)
    {
        loopv(projs)
        {
            projectile &p = projs[i];
            obstacles.avoidnear(NULL, p.o.z + skills[p.atk].exprad + 1, p.o, radius + skills[p.atk].exprad);
        }
    }

    void onplayerdoubletap( physent* pl ) {
        gameent *ge = (gameent*)pl;

        if( ge->state != CS_ALIVE
        || ge->timeinair ) {
            ge->k_doubletaplast = lastmillis;
            ge->k_doubletapid = -1;
            return;
        }
        
        int move = 0, strafe = 0;

        switch( ge->k_doubletapid ) {
            case 1: move = -1; break;
            case 2: move = 1; break;
            case 3: strafe = 1; break;
            case 4: strafe = -1; break;
        }

        vec m(0.0f, 0.0f, 0.0f);
        if( move || strafe )
        {
            vecfromyawpitch(pl->yaw, 0, move, strafe, m);

            /*if(!floating && pl->physstate >= PHYS_SLOPE)
            {
                float dz = -(m.x*pl->floor.x + m.y*pl->floor.y)/pl->floor.z;
                m.z = water ? max(m.z, dz) : dz;
            }*/

            m.normalize();
        }

        vec d(m);
        d.mul( DODGE_SPEED * DODGE_MULIPLIER );

        ge->vel.add( d );

        ge->dodgetime = lastmillis + DODGE_TIME;

        if( m_singleplayer ) {
            execident( "playerdodge" );
        }
    }
};

