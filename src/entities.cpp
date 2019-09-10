#include "game.h"

#if !STANDALONE
namespace UI {
    bool uivisible(const char *name);
    bool showui(const char *name);
    bool hideui(const char *name);
}
#endif //!STANDALONE

gameentity::gameentity() {
    actiontime = 0;
    uihud = false;
}

namespace entities
{
    using namespace game;

    void local_drop( gameent *d, int entnum ) {
        vec o = d->feetpos();
        extentity &e = *ents[ entnum ];
        e.flags &= ~EF_NOCOLLIDE;
        e.flags &= ~EF_NORENDER;
        e.o = o;
        e.attr2 = d->yaw;

#if !STANDALONE
        mpeditent( entnum, o, e.type, e.attr1, e.attr2, e.attr3, e.attr4, e.attr5, false);
#endif //!STANDALONE

        d->grabentity = -1;
        d->playermod = 0;
        d->customspeed = 0;
        d->waitdrop = false;
    }

    void drop( gameent *d ) {
        if( d->grabentity == -1
        || d->waitdrop ) {
            return;
        }

        vec npos = d->feetpos();

        extentity &e = *ents[ d->grabentity ];
#if !STANDALONE
        addmsg( N_ITEMDROP, "ri2i3ii5", d->clientnum, d->grabentity, (int)(npos.x*DMF), (int)(npos.y*DMF), (int)(npos.z*DMF), e.type, e.attr1, (int)( d->yaw ), e.attr3, e.attr4, e.attr5 );
#endif //!STANDALONE
        d->waitdrop = true;
    }

    int extraentinfosize() { return 0; }       // size in bytes of what the 2 methods below read/write... so it can be skipped by other games

    void writeent(entity &e, char *buf)   // write any additional data to disk (except for ET_ ents)
    {
    }

    void readent(entity &e, char *buf, int ver)     // read from disk, and init
    {
    }

#if 1
    vector<extentity *> ents;

    vector<extentity *> &getents() { return ents; }

    bool mayattach(extentity &e) { return false; }
    bool attachent(extentity &e, extentity &a) { return false; }

    const char *itemname(int i)
    {
        return NULL;
#if 0
        int t = ents[i]->type;
        if(!validitem(t)) return NULL;
        return itemstats[t-I_FIRST].name;
#endif
    }

    int itemicon(int i)
    {
        return -1;
#if 0
        int t = ents[i]->type;
        if(!validitem(t)) return -1;
        return itemstats[t-I_FIRST].icon;
#endif
    }

    const char *entmdlname(int type)
    {
        if( gameentities[ type ].mdl == -1 ) {
            return 0;
        }

        return models[ gameentities[ type ].mdl ].name;
    }

#if !STANDALONE
    const char *entmodel(const entity &e) {
        if(e.type == TELEPORT)
        {
            if(e.attr2 > 0) return mapmodelname(e.attr2);
            if(e.attr2 < 0) return NULL;
        }
        return e.type < MAXENTTYPES ? entmdlname(e.type) : NULL;
    }
#endif //!STANDALONE

    void preloadentities()
    {
        loopv(ents)
        {
            extentity &e = *ents[i];
            setup( e );
        }
    }

    void renderentities() {
#if !STANDALONE
        loopv(ents)
        {
            extentity &e = *ents[i];

            if( ( e.flags & EF_NORENDER ) != 0 ) {
                continue;
            }
            int revs = 10;
            switch(e.type)
            {
                case CANNON:
                case TELEPORT:
                    if(e.attr2 < 0) continue;
                    break;
                default:
                    if(!e.spawned() || !validitem(e.type)) continue;
                    break;
            }
            const char *mdlname = entmodel(e);
            if(mdlname)
            {
                modelattach a[5];
                int ai = 0;
                vec p = e.o;
                bool animate = gameentities[ e.type ].animate;
                float yaw = 0.0f;
                int flags = MDL_CULL_VFC | MDL_CULL_DIST | MDL_CULL_OCCLUDED;
                int animflags = ANIM_MAPMODEL | ANIM_LOOP;
                int basetime = 0;
                gameentity &ge = *(gameentity *)entities::ents[i];
                if( e.type == CANNON ) {
                    a[ai++] = modelattach( "tag_muzzle", &ge.muzzle );
                    a[ai++] = modelattach( "tag_powder", &ge.powder );
                    yaw = e.attr2;
                    if( lastmillis - ge.actiontime < 833 ) {
                        basetime = ge.actiontime;
                        animflags = gameentities[ e.type ].animaction;
                    }
                    else {
                        animflags = gameentities[ e.type ].animidle;
                    }
                    animflags |= ANIM_LOOP;
                }
                
                if( animate ) {
                    yaw = lastmillis/(float)revs;
                    p.z += 1+sinf(lastmillis/100.0+e.o.x+e.o.y)/20;
                }
                else {
                    yaw = e.attr2;
                }

                rendermodel(mdlname, animflags, p, yaw, 0, 0, flags, 0, a[0].tag ? a : NULL, basetime, 0, 1.0f, vec4(vec::hexcolor(0xFFFFFFFF), 1.0f));
            }
        }
#endif //!STANDALONE
    }

    void addammo(int type, int &v, bool local)
    {
#if 0
        itemstat &is = itemstats[type-I_FIRST];
        v += is.add;
        if(v>is.max) v = is.max;
        if(local) msgsound(is.sound);
#endif
    }

    // these two functions are called when the server acknowledges that you really
    // picked up the item (in multiplayer someone may grab it before you).

    void pickupeffects(int n, gameent *d)
    {
#if 0
        if(!ents.inrange(n)) return;
        int type = ents[n]->type;
        if(!validitem(type)) return;
        ents[n]->clearspawned();
        if(!d) return;
        itemstat &is = itemstats[type-I_FIRST];
        if(d!=player1 || isthirdperson())
        {
            //particle_text(d->abovehead(), is.name, PART_TEXT, 2000, 0xFFC864, 4.0f, -8);
            particle_icon(d->abovehead(), is.icon%4, is.icon/4, PART_HUD_ICON_GREY, 2000, 0xFFFFFF, 2.0f, -8);
        }
        playsound(itemstats[type-I_FIRST].sound, d!=player1 ? &d->o : NULL, NULL, 0, 0, 0, -1, 0, 1500);
        d->pickup(type);
        if(d==player1) switch(type)
        {
        }
#endif
    }

    // these functions are called when the client touches the item

    void teleporteffects(gameent *d, int tp, int td, bool local)
    {
#if !STANDALONE
        if(ents.inrange(tp) && ents[tp]->type == TELEPORT)
        {
            extentity &e = *ents[tp];
            if(e.attr4 >= 0)
            {
                int snd = S_TELEPORT, flags = 0;
                if(e.attr4 > 0) { snd = e.attr4; flags = SND_MAP; }
                if(d == player1) playsound(snd, NULL, NULL, flags);
                else
                {
                    playsound(snd, &e.o, NULL, flags);
                    if(ents.inrange(td) && ents[td]->type == TELEDEST) playsound(snd, &ents[td]->o, NULL, flags);
                }
            }
        }
        if(local && d->clientnum >= 0)
        {
            sendposition(d);
            packetbuf p(32, ENET_PACKET_FLAG_RELIABLE);
            putint(p, N_TELEPORT);
            putint(p, d->clientnum);
            putint(p, tp);
            putint(p, td);
            sendclientpacket(p.finalize(), 0);
            flushclient();
        }
#endif //!STANDALONE
    }

    void jumppadeffects(gameent *d, int jp, bool local)
    {
#if !STANDALONE
        if(ents.inrange(jp) && ents[jp]->type == JUMPPAD)
        {
            extentity &e = *ents[jp];
            if(e.attr4 >= 0)
            {
                int snd = S_JUMPPAD, flags = 0;
                if(e.attr4 > 0) { snd = e.attr4; flags = SND_MAP; }
                if(d == player1) playsound(snd, NULL, NULL, flags);
                else playsound(snd, &e.o, NULL, flags);
            }
        }
        if(local && d->clientnum >= 0)
        {
            sendposition(d);
            packetbuf p(16, ENET_PACKET_FLAG_RELIABLE);
            putint(p, N_JUMPPAD);
            putint(p, d->clientnum);
            putint(p, jp);
            sendclientpacket(p.finalize(), 0);
            flushclient();
        }
#endif //!STANDALONE
    }

    void teleport(int n, gameent *d)     // also used by monsters
    {
#if !STANDALONE
        int e = -1, tag = ents[n]->attr1, beenhere = -1;
        for(;;)
        {
            e = findentity(TELEDEST, e+1);
            if(e==beenhere || e<0) { conoutf(CON_WARN, "no teleport destination for tag %d", tag); return; }
            if(beenhere<0) beenhere = e;
            if(ents[e]->attr2==tag)
            {
                teleporteffects(d, n, e, true);
                d->o = ents[e]->o;
                d->yaw = ents[e]->attr1;
                if(ents[e]->attr3 > 0)
                {
                    vec dir;
                    vecfromyawpitch(d->yaw, 0, 1, 0, dir);
                    float speed = d->vel.magnitude2();
                    d->vel.x = dir.x*speed;
                    d->vel.y = dir.y*speed;
                }
                else d->vel = vec(0, 0, 0);
                entinmap(d);
                updatedynentcache(d);
                ai::inferwaypoints(d, ents[n]->o, ents[e]->o, 16.f);
                break;
            }
        }
#endif //!STANDALONE
    }

    VARR(teleteam, 0, 1, 1);

    void trypickup(int n, gameent *d) {
#if !STANDALONE
        extentity *e = ents[n];
        gameentity &ge = *(gameentity *)e;
        
        if( gameentities[ e->type ].canbegrab ) {
            int activatetime = lastmillis-d->lastactivate;
            if( activatetime < 500 ) {
                if( !classes[ d->classid ].cangrab ) return;
                if(d->lastpickup==ents[n]->type && lastmillis-d->lastpickupmillis<1000) return;
                d->lastpickup = ents[n]->type;
                d->lastpickupmillis = lastmillis;
                d->lastactivate = 0;

                addmsg( N_GRAB_ENTITY, "rii", d->clientnum, n );
                return;
            }
        }

#if !STANDALONE
        if( !UI::uivisible( 0 ) ) {
            defformatstring( hudname, "hud_%s", gameentities[ ents[n]->type ].name );
            UI::showui( hudname );
            ge.uihud = true;
        }
#endif //!STANDALONE

        switch(ents[n]->type)
        {
            default:
                if(d->canpickup(ents[n]->type))
                {
                    addmsg(N_ITEMPICKUP, "rci", d, n);
                    ents[n]->clearspawned(); // even if someone else gets it first
                }
                break;

            case TELEPORT:
            {
                if(d->lastpickup==ents[n]->type && lastmillis-d->lastpickupmillis<500) break;
                if(!teleteam && m_teammode) break;
                if(ents[n]->attr3 > 0)
                {
                    defformatstring(hookname, "can_teleport_%d", ents[n]->attr3);
                    if(!execidentbool(hookname, true)) break;
                }
                d->lastpickup = ents[n]->type;
                d->lastpickupmillis = lastmillis;
                teleport(n, d);
                break;
            }

            case JUMPPAD:
            {
                if(d->lastpickup==ents[n]->type && lastmillis-d->lastpickupmillis<300) break;
                d->lastpickup = ents[n]->type;
                d->lastpickupmillis = lastmillis;
                jumppadeffects(d, n, true);
                if(d->ai) d->ai->becareful = true;
                d->falling = vec(0, 0, 0);
                d->physstate = PHYS_FALL;
                d->timeinair = 1;
                d->vel = vec(ents[n]->attr3*10.0f, ents[n]->attr2*10.0f, ents[n]->attr1*12.5f);
                break;
            }

            case LADDER: {
                d->onladder = 1;
                break;
            }

            case CANNON: {
                int attacktime = lastmillis-d->gunstarttime;
                if( attacktime < 500 ) {
                    if(d->lastpickup==ents[n]->type && lastmillis-d->lastpickupmillis<1000) break;
                    d->lastpickup = ents[n]->type;
                    d->lastpickupmillis = lastmillis;

                    addmsg( N_USE_ENTITY, "rii", d->clientnum, n );
                }
                break;
            }

            case POINTOFINTEREST: {
                if(d->lastpickup==ents[n]->type && lastmillis-d->lastpickupmillis<1000) break;
                d->lastpickup = ents[n]->type;
                d->lastpickupmillis = lastmillis;
                defformatstring( identname, "pointofinterest%d", n);
                execident( identname );
                break;
            }
        }
#endif //!STANDALONE
    }
    
    void effects( int n, gameent *d ) {
#if !STANDALONE
        extentity *e = ents[n];
 
        switch( e->type ) {
            case CANNON: {
                vec dir;
                vecfromyawpitch( e->attr2, 0, 1, 0, dir);

                // go a bit higher
                dir.z += 0.1f;

                gameentity &ge = *(gameentity *)e;
                vec p = ge.muzzle;

                ge.actiontime = lastmillis;

                particle_splash(PART_SPARK, 10, 100, ge.powder, 0.45f);
                vec vcol( 1.0f, 1.0f, 1.0f );
                adddynlight( ge.powder, 35, vcol, 75, 75 );
                adddynlight( ge.muzzle, 35, vcol, 75, 75 );

                vec smokevec( dir );
                smokevec.mul( 20 );
                vec smoketo( ge.muzzle );
                smoketo.add( smokevec );

                particle_flare(ge.muzzle, smoketo, 400, PART_SMOKE, 0x000000, 5.50f, d);
                particle_trail(PART_STEAM, 50, smoketo, ge.muzzle, 0x000000, /*float size =*/ 2.0f, /*int gravity =*/ 20);
                particle_flare(ge.muzzle, smoketo, 200, PART_EXPLOSION, 0x303020, 3.0f, d);

                if( d == player1 ) {
                    playsound(skills[ATK_CANNON].hudsound, NULL);
                }
                else {
                    playsound(skills[ATK_CANNON].sound, &d->o);
                }

                vec faraway( dir );
                faraway.mul( 100 );
                vec to( p );
                to.add( faraway );
                newbouncer(p, to, true, 0, d, BNC_GRENADE, 2000, 400, ATK_CANNON);
                break;
            }
        }
#endif //!STANDALONE
    }

    void local_grab( int n, gameent *d ) {
#if !STANDALONE
        extentity &e = *ents[n];

        if( gameentities[ e.type ].canbegrab ) {
            e.flags |= EF_NOCOLLIDE |  EF_NORENDER;
            if( d == player1 ) {
                thirdperson = 0;
            }
            d->grabentity = n;
            d->playermod = gameentities[ e.type ].grabplayermod;
            d->customspeed = gameentities[ e.type ].grabspeed;
        }
#endif //!STANDALONE
    }

    void checkitems(gameent *d)
    {
        if(d->state!=CS_ALIVE) return;

        vec o = d->feetpos();
        d->onladder = 0;

        if( d->grabentity != -1 ) {
            extentity &e = *ents[d->grabentity];
            int activatetime = lastmillis-d->lastactivate;
            if( d->timeinair == 0
            && activatetime < 500 ) {
                drop( d );
                d->lastactivate = 0;
            }
            return;
        }

        loopv(ents) {
            extentity &e = *ents[i];
            if(e.type==NOTUSED) continue;
            if( !e.spawned()
            && e.type!=TELEPORT
            && e.type!=JUMPPAD
            && e.type!=LADDER
            && e.type != CANNON
            && e.type != POINTOFINTEREST) {
                continue;
            }
            if( (e.flags & EF_NORENDER) ) {
                continue;
            }
            gameentity &ge = *(gameentity *)&e;
            float entdist = gameentities[ e.type ].distance;
            bool closeui = false;
            if( e.type == CANNON ) {
                o = d->headpos();
                float dist = ge.powder.dist(o);
                if(dist<entdist) {
                    trypickup(i, d);
                }
                else {
                    closeui = true;
                }
            }
            else {
                float dist = e.o.dist(o);
                if(dist<entdist) { 
                    trypickup(i, d);
                }
                else {
                    closeui = true;
                }
            }

#if !STANDALONE
            if( closeui
            && ge.uihud ) {
                defformatstring( hudname, "hud_%s", gameentities[ ge.type ].name );
                UI::hideui( hudname );
                ge.uihud = false;
            }
#endif //!STANDALONE
        }
    }

    void putitems(packetbuf &p)            // puts items in network stream and also spawns them locally
    {
        putint(p, N_ITEMLIST);
        loopv(ents) if(validitem(ents[i]->type))
        {
            extentity &e = *ents[i];
            int grabbed = ( e.flags & EF_NORENDER );

            putint(p, i);
            putint(p, e.type);
            putint(p, grabbed);
            putint(p, e.o.x*DMF);
            putint(p, e.o.y*DMF);
            putint(p, e.o.z*DMF);
            putint(p, e.attr1);
            putint(p, e.attr2);
            putint(p, e.attr3);
            putint(p, e.attr4);
            putint(p, e.attr5);
        }
        putint(p, -1);
    }

    void resetspawns() { loopv(ents) ents[i]->clearspawned(); }

    void spawnitems(bool force)
    {
        loopv(ents) if(validitem(ents[i]->type))
        {
            ents[i]->setspawned(force || !server::delayspawn(ents[i]->type));
        }
    }

    void update( int i, bool grabbed, float x, float y, float z, int attr1, int attr2, int attr3, int attr4, int attr5 ) {
#if !STANDALONE
        if(ents.inrange(i)) {
            extentity &e = *ents[i];
            e.o = vec( x, y, z );
            e.attr1 = attr1;
            e.attr2 = attr2;
            e.attr3 = attr3;
            e.attr4 = attr4;
            e.attr5 = attr5;

            if( gameentities[ e.type ].canbegrab ) {
                if( grabbed ) {
                    e.flags |= EF_NOCOLLIDE |  EF_NORENDER;
                }
                else {
                    e.flags &= ~EF_NOCOLLIDE;
                    e.flags &= ~EF_NORENDER;
                }
            }

            mpeditent( i, e.o, e.type, e.attr1, e.attr2, e.attr3, e.attr4, e.attr5, false);
        }
#endif //!STANDALONE
    }

    void setspawn(int i, bool on) { if(ents.inrange(i)) ents[i]->setspawned(on); }

    extentity *newentity() { return new gameentity(); }
    void deleteentity(extentity *e) { delete (gameentity *)e; }

    void clearents()
    {
        while(ents.length()) deleteentity(ents.pop());
    }

    void animatemapmodel(const extentity &e, int &anim, int &basetime)
    {
    }

    void fix(extentity &e)
    {
#if !STANDALONE
        switch(e.type)
        {
            case FLAG:
                e.attr5 = e.attr4;
                e.attr4 = e.attr3;
            case TELEDEST:
                e.attr3 = e.attr2;
                e.attr2 = e.attr1;
                e.attr1 = (int)player1->yaw;
                break;
        }
#endif //!STANDALONE
    }

    void setup(extentity &e)
    {
#if !STANDALONE
        switch(e.type)
        {
            case MMSTART:
            case POINTOFINTEREST:
            case CAPTUREPOINT:
            case LADDER:
            case FLAG:
            case TELEDEST:
                e.flags |= EF_NOCOLLIDE;
                break;
            case TELEPORT:
                if(e.attr2 > 0) preloadmodel(mapmodelname(e.attr2));
            case JUMPPAD:
                if(e.attr4 > 0) preloadmapsound(e.attr4);
            default:
                break;
        }
#endif //!STANDALONE
    }

    void entradius(extentity &e, bool color)
    {
#if !STANDALONE
        switch(e.type)
        {
            case TELEPORT:
                loopv(ents) if(ents[i]->type == TELEDEST && e.attr1==ents[i]->attr2)
                {
                    renderentarrow(e, vec(ents[i]->o).sub(e.o).normalize(), e.o.dist(ents[i]->o));
                    break;
                }
                break;
            
            case LADDER:
                renderentarrow(e, vec((int)(char)e.attr3*10.0f, (int)(char)e.attr2*10.0f, e.attr1*12.5f).normalize(), 4);
                break;

            case JUMPPAD:
                renderentarrow(e, vec((int)(char)e.attr3*10.0f, (int)(char)e.attr2*10.0f, e.attr1*12.5f).normalize(), 4);
                break;

            case CANNON: {
                vec dir;
                vecfromyawpitch(e.attr2, 0, 1, 0, dir);
                renderentarrow(e, dir, 20);
                break;
            }

            case POINTOFINTEREST: {
                renderentsphere( e, gameentities[ e.type ].distance );
                break;
            }

            case MMSTART: {
                vec dir;
                vecfromyawpitch(e.attr1, e.attr2, 1, 0, dir);
                renderentarrow(e, dir, 4);
                break;
            }
            case CAPTUREPOINT:
            case FLAG:
            case TELEDEST:
            {
                vec dir;
                vecfromyawpitch(e.attr1, 0, 1, 0, dir);
                renderentarrow(e, dir, 4);
                break;
            }
        }
#endif //!STANDALONE
    }

    bool printent(extentity &e, char *buf, int len)
    {
        return false;
    }

    const char *entnameinfo(entity &e) { return ""; }
    const char *entname(int i)
    {
        return i>=0 && i<MAXENTTYPES ? gameentities[i].name : "";
    }

    void editent(int i, bool local)
    {
#if !STANDALONE
        extentity &e = *ents[i];
        //e.flags = 0;
        if(local) addmsg(N_EDITENT, "rii3ii5", i, (int)(e.o.x*DMF), (int)(e.o.y*DMF), (int)(e.o.z*DMF), e.type, e.attr1, e.attr2, e.attr3, e.attr4, e.attr5);
#endif //!STANDALONE
    }

    float dropheight(entity &e)
    {
        if( e.type==FLAG || e.type == CAPTUREPOINT ) {
            return 0.0f;
        }
        return 4.0f;
    }
#endif
}

