#include "game.h"
#include "engine.h"

extern selinfo sel;

namespace game
{
    vector<gameent *> bestplayers;
    vector<int> bestteams;

    VARP(ragdoll, 0, 1, 1);
    VARP(ragdollmillis, 0, 10000, 300000);
    VARP(ragdollfade, 0, 100, 5000);
    VARP(forceplayermodels, 0, 0, 1);
    VARP(hidedead, 0, 0, 1);
    VAR(showhitpoint, 0, 0, 20000);
    VAR(shownames, 0, 1, 1);
    VAR(showbounds, 0, 0, 1);
    extern int hasweapons;

    SVAR(editguidemodel, "");
    VAR(editguidemove, 0, 0, 1);
    FVAR(editguidealpha, 0, 0.5, 1);
    static vec editguidepos( 0.0f, 0.0f, 0.0f );

    static bool iscustomcamera() {
        return !thirdperson && ( player1->state == CS_ALIVE || player1->state == CS_SPECTATOR );
    }

    extern int playermodel;

    vector<gameent *> ragdolls;

    void saveragdoll(gameent *d)
    {
        if(!d->ragdoll || !ragdollmillis || (!ragdollfade && lastmillis > d->lastpain + ragdollmillis)) return;
        gameent *r = new gameent(*d);
        r->lastupdate = ragdollfade && lastmillis > d->lastpain + max(ragdollmillis - ragdollfade, 0) ? lastmillis - max(ragdollmillis - ragdollfade, 0) : d->lastpain;
        r->edit = NULL;
        r->ai = NULL;
        ragdolls.add(r);
        d->ragdoll = NULL;
    }

    void clearragdolls()
    {
        ragdolls.deletecontents();
    }

    void moveragdolls()
    {
        loopv(ragdolls)
        {
            gameent *d = ragdolls[i];
            if(lastmillis > d->lastupdate + ragdollmillis)
            {
                delete ragdolls.remove(i--);
                continue;
            }
            moveragdoll(d);
        }
    }

    void preloadmodels()
    {
        loopi( NUMMODELS ) {
            const modelinfo& mdl = models[i];
            preloadmodel( mdl.name );
        }
    }

    int numanims() { return NUMANIMS; }

    void findanims(const char *pattern, vector<int> &anims)
    {
        loopi(sizeof(animnames)/sizeof(animnames[0])) if(matchanim(animnames[i], pattern)) anims.add(i);
    }

    VAR(animoverride, -1, 0, NUMANIMS-1);
    VAR(testanims, 0, 0, 1);
    VAR(testpitch, -90, 0, 90);

    void renderplayer(gameent *d, const modelinfo &body, const modelinfo &weapon, int team, float fade, int flags = 0, bool mainpass = true)
    {
        bool isplayer = d == player1;
        bool isskillcooldown = lastmillis - d->gunstarttime < d->gunwait;
        int lastaction = d->lastaction
        , anim = ANIM_IDLE|ANIM_LOOP
        , attack = 0
        , delay = 0
        , secondarymodel = -1
        , secondary = -1;
        if( d->lastskill != NO_SKILL )
        {
            attack = skills[d->lastskill].anim;
            delay = skills[d->lastskill].attackdelay+50;
            secondarymodel = skills[d->lastskill].secondarymodel;
        }
        if(intermission && d->state!=CS_DEAD)
        {
            anim = attack = ANIM_LOSE|ANIM_LOOP;
            if(validteam(team) ? bestteams.htfind(team)>=0 : bestplayers.find(d)>=0) anim = attack = ANIM_WIN|ANIM_LOOP;
        }
        else if(d->state==CS_ALIVE && d->lasttaunt && lastmillis-d->lasttaunt<1000 && lastmillis-d->lastaction>delay)
        {
            lastaction = d->lasttaunt;
            anim = attack = ANIM_TAUNT;
            delay = 1000;
        }

        const char *mdlname = body.name;
        float yaw = testanims && d==player1 ? 0 : d->yaw,
              pitch = testpitch && d==player1 ? testpitch : d->pitch;
        vec o = d->feetpos();
        int basetime = 0;
        if(animoverride) anim = (animoverride<0 ? ANIM_ALL : animoverride)|ANIM_LOOP;
        else if(d->state==CS_DEAD)
        {
            anim = ANIM_DYING|ANIM_NOPITCH;
            basetime = d->lastpain;
            if(ragdoll && body.ragdoll) anim |= ANIM_RAGDOLL;
            else if(lastmillis-basetime>1000) anim = ANIM_DEAD|ANIM_LOOP|ANIM_NOPITCH;
        }
        else if(d->state==CS_EDITING || d->state==CS_SPECTATOR) anim = ANIM_EDIT|ANIM_LOOP;
        else if(d->state==CS_LAGGED)                            anim = ANIM_LAG|ANIM_LOOP;
        else if(!intermission)
        {
            if( d->stuntime > lastmillis ) {
                anim = ANIM_STUN;
                basetime = d->laststun;
            }
            else if(lastmillis-d->lastpain < 300)
            {
                anim = ANIM_PAIN;
                basetime = d->lastpain;
            }
            else if( !isskillcooldown && d->nextskill != NO_SKILL ) {
                int atk = d->nextskill;
                anim = skills[ atk ].stillanim;
                basetime = 0;
            }
            else if( lastmillis-lastaction < delay)
            {
                anim = attack;
                basetime = lastaction;
                secondary = secondarymodel;
            }

            if(d->inwater && d->physstate<=PHYS_FALL) anim |= (((game::allowmove(d) && (d->move || d->strafe)) || d->vel.z+d->falling.z>0 ? ANIM_SWIM : ANIM_SINK)|ANIM_LOOP)<<ANIM_SECONDARY;
            else
            {
                static const int dirs[9] =
                {
                    ANIM_RUN_SE, ANIM_RUN_S, ANIM_RUN_SW,
                    ANIM_RUN_NE,  0,          ANIM_RUN_NW,
                    ANIM_RUN_NE, ANIM_RUN_N, ANIM_RUN_NW
                };
                int diridx = (d->move+1)*3 + (d->strafe+1);
                int dir = dirs[diridx];
                if( d->timeinair>100 ) anim |= ((dir ? dir+ANIM_JUMP_N-ANIM_RUN_N : ANIM_JUMP) | ANIM_END) << ANIM_SECONDARY;
                else if( d->grabentity != -1 ) {
                    int grabtype = entities::getents()[ d->grabentity ]->type;
                    int grabanim = d->move+d->strafe != 0
                    ? gameentities[ grabtype ].animplayergrabwalk
                    : gameentities[ grabtype ].animplayergrabidle;
                    anim |= ( grabanim | ANIM_LOOP) << ANIM_SECONDARY;
                }
                else if( d->dodgetime > lastmillis ) { 
                    static const int dodgedirs[9] =
                    {
                        ANIM_PLAYER_DODGE_S, ANIM_PLAYER_DODGE_S, ANIM_PLAYER_DODGE_S,
                        ANIM_PLAYER_DODGE_NE,  0,          ANIM_PLAYER_DODGE_NW,
                        ANIM_PLAYER_DODGE_NE, ANIM_PLAYER_DODGE_N, ANIM_PLAYER_DODGE_NW
                    };
                    anim |= ( dodgedirs[diridx] | ANIM_END) << ANIM_SECONDARY;
                }
                else if(dir && game::allowmove(d)) anim |= (dir | ANIM_LOOP) << ANIM_SECONDARY;
            }

            if(d->crouching) switch((anim>>ANIM_SECONDARY)&ANIM_INDEX)
            {
                case ANIM_IDLE: anim &= ~(ANIM_INDEX<<ANIM_SECONDARY); anim |= ANIM_CROUCH<<ANIM_SECONDARY; break;
                case ANIM_JUMP: anim &= ~(ANIM_INDEX<<ANIM_SECONDARY); anim |= ANIM_CROUCH_JUMP<<ANIM_SECONDARY; break;
                case ANIM_SWIM: anim &= ~(ANIM_INDEX<<ANIM_SECONDARY); anim |= ANIM_CROUCH_SWIM<<ANIM_SECONDARY; break;
                case ANIM_SINK: anim &= ~(ANIM_INDEX<<ANIM_SECONDARY); anim |= ANIM_CROUCH_SINK<<ANIM_SECONDARY; break;
                case 0: anim |= (ANIM_CROUCH|ANIM_LOOP)<<ANIM_SECONDARY; break;
                case ANIM_RUN_N: case ANIM_RUN_NE: case ANIM_RUN_E: case ANIM_RUN_SE: case ANIM_RUN_S: case ANIM_RUN_SW: case ANIM_RUN_W: case ANIM_RUN_NW:
                    anim += (ANIM_CROUCH_N - ANIM_RUN_N) << ANIM_SECONDARY;
                    break;
                case ANIM_JUMP_N: case ANIM_JUMP_NE: case ANIM_JUMP_E: case ANIM_JUMP_SE: case ANIM_JUMP_S: case ANIM_JUMP_SW: case ANIM_JUMP_W: case ANIM_JUMP_NW:
                    anim += (ANIM_CROUCH_JUMP_N - ANIM_JUMP_N) << ANIM_SECONDARY;
                    break;
            }

            if((anim&ANIM_INDEX)==ANIM_IDLE && (anim>>ANIM_SECONDARY)&ANIM_INDEX) anim >>= ANIM_SECONDARY;
        }
        if(!((anim>>ANIM_SECONDARY)&ANIM_INDEX)) anim |= (ANIM_IDLE|ANIM_LOOP)<<ANIM_SECONDARY;
        if(d!=player1 && followingplayer() != d ) flags |= MDL_CULL_VFC | MDL_CULL_OCCLUDED | MDL_CULL_QUERY;
        if(d->type==ENT_PLAYER) flags |= MDL_FULLBRIGHT;
        else flags |= MDL_CULL_DIST;
        //if(!mainpass) flags &= ~(MDL_FULLBRIGHT | MDL_CULL_VFC | MDL_CULL_OCCLUDED | MDL_CULL_QUERY | MDL_CULL_DIST);
        modelattach a[7];
        int ai = 0;

        if( ( isplayer || followingplayer() == d ) && usecustomcamera ) {
            if( !mainpass ) {
                cameraorigin = player1->o;
                a[ai++] = modelattach( "tag_camera", &cameraorigin );
            }
        }

        if( hasweapons
        && weapon.name )
        {
            a[ai++] = modelattach( "tag_weapon", weapon.name );
        }

        if( secondary != -1 ) {
            a[ai++] = modelattach( "tag_secondary", models[ secondary ].name );
        }

        if( mainpass ) {
            a[ai++] = modelattach( "tag_muzzle", &d->muzzle );
            a[ai++] = modelattach( "tag_powder", &d->powder );
            a[ai++] = modelattach( "tag_head", &d->taghead );

            if( d->grabentity != -1 ) {
                int grabtype = entities::getents()[ d->grabentity ]->type;
                const char *mdl = models[ gameentities[ grabtype ].mdl3 ].name;
                const char *tagname = gameentities[ grabtype ].grabattachname;
                if( tagname && mdl ) {
                    a[ai++] = modelattach( tagname, mdl );
                }
            }
        }

        float trans =
        d->state == CS_LAGGED ? 0.5f :
        d->cloakmode == 1 ? 0.5f
        : 1.0f;
        rendermodel(mdlname, anim, o, yaw, pitch, 0, flags, d, a[0].tag ? a : NULL, basetime, 0, fade, vec4(vec::hexcolor(0xFFFFFFFF), trans));
    }

    static inline void renderplayer(gameent *d, float fade = 1, int flags = 0, bool mainpass = true )
    {
        int team = m_teammode && validteam(d->team) ? d->team : 0;
        const modelinfo &body = models[ classes[ d->classid ].model ];
        const modelinfo &weapon = models[ classes[ d->classid ].weapon ];
        renderplayer( d, body, weapon, team, fade, flags, mainpass );
    }

    void renderguide() {
        if( editguidemodel[0] != 0 ) {
            if( editguidemove ) {
                editguidepos = vec( sel.o );
            }

            rendermodel( editguidemodel, ANIM_MAPMODEL|ANIM_LOOP, editguidepos, /*yaw*/0, /*pitch*/0, 0, MDL_WIREFRAME|MDL_FULLBRIGHT, player1, 0, 0, 0, 1.0f, vec4(vec::hexcolor(0xFFFFFFFF), editguidealpha));
        }
    }

    void rendergame()
    {
        ai::render();

        if(intermission)
        {
            bestteams.shrink(0);
            bestplayers.shrink(0);
            if(m_teammode) getbestteams(bestteams);
            else getbestplayers(bestplayers);
        }

        gameent *f = followingplayer();
        if( !mainmenu )
        loopv(players)
        {
            gameent *d = players[i];

            if( d != player1
            && d->state == CS_EDITING ) {
                copystring(d->info, colorname(d));
                particle_splash(PART_SPARK, 10, 10, d->o, 0.45f);
                particle_text(d->abovehead(), d->info, PART_TEXT, 1, teams[ 0 ].textcolor, 2.0f);
                continue;
            }

            if( d->state == CS_EDITING
            || d->state==CS_SPECTATOR
            || d->state==CS_SPAWNING
            || d->lifesequence < 0
            || (d->state==CS_DEAD && hidedead)) {
                continue;
            }

            renderplayer(d);

            if( showbounds ) {
                vec bottom(d->o), top(d->o);
                bottom.z -= d->eyeheight;
                top.z += d->aboveeye;

                bottom = d->feetpos();
                top = d->headpos();
                particle_splash(PART_SPARK, 10, 10, bottom, 0.45f);
                particle_splash(PART_SMOKE, 10, 10, top, 0.45f);
            }

            if( d != player1 && shownames ) {
                copystring(d->info, colorname(d));
                if(d->state!=CS_DEAD)
                {
                    int team = m_teammode && validteam(d->team) ? d->team : 0;

                    if( lastmillis - d->lastpainfromlocal < 5000 ) {
                        int x = 3;
                        if( d->health > 75 ) {
                            x = 0;
                        }
                        else if( d->health > 50 ) {
                            x = 1;
                        }
                        else if( d->health > 25 ) {
                            x = 2;
                        }
                        particle_icon(d->abovehead(), x, 1, PART_HUD_ICON, 1, 0xFFFFFF, 2.0f, 0);
                    }
                    else {
                        particle_text(d->abovehead(), d->info, PART_TEXT, 1, teams[team].textcolor, 2.0f);
                    }
                }
            }


            if( showhitpoint ) {
                vec dir;
                vecfromyawpitch(d->yaw, d->pitch, 1, 0, dir);
                dir.mul( showhitpoint );
                vec to = d->o;
                to.add( dir );
                particle_splash(PART_SPARK, 10, 10, to, 0.45f);
            }
        }
        loopv(ragdolls)
        {
            gameent *d = ragdolls[i];
            float fade = 1.0f;
            if(ragdollmillis && ragdollfade)
                fade -= clamp(float(lastmillis - (d->lastupdate + max(ragdollmillis - ragdollfade, 0)))/min(ragdollmillis, ragdollfade), 0.0f, 1.0f);
            renderplayer(d, fade);
        }
        entities::renderentities();
        renderbouncers();
        renderprojectiles();
        if(cmode) cmode->rendergame();
        renderguide();
    }

    void renderavatar() {
    }

    void renderplayerpreview( int classid )
    {
        const modelinfo &body = models[ classes[ classid ].model ];
        const modelinfo &weapon = models[ classes[ classid ].weapon ];

        int anim = ANIM_IDLE | ANIM_LOOP;
        model *m = loadmodel( body.name );
        if(m)
        {
            vec center, radius;
            m->boundbox(center, radius);

            // TODO: remove this please
            radius = vec( 7.9f, 1.8f, 9.0f );

            float yaw;
            vec o = calcmodelpreviewpos(radius, yaw).sub(center);

            modelattach a[6];
            int ai = 0;

            if( weapon.name )
            {
                a[ai++] = modelattach( "tag_weapon", weapon.name );
            }

            rendermodel( body.name, anim, o, yaw, 0, 0, 0, 0, a[0].tag ? a : 0 );
        }
    }

    void preloadsounds()
    {
        for(int i = S_JUMP; i <= S_DIE2; i++) preloadsound(i);
    }

    void preload()
    {
        preloadbouncers();
        preloadmodels();
        preloadsounds();
        entities::preloadentities();
    }

    void preframe() {
        if( connected ) {
            usecustomcamera = iscustomcamera();
            gameent *d = player1;
            if( usecustomcamera && player1->state == CS_SPECTATOR ) {
                gameent *f = followingplayer();
                if( f ) {
                    d = f;
                    camera1->yaw = d->yaw;
                    camera1->pitch = d->pitch;
                }
            }
            renderplayer( d, 1, MDL_NOBATCH | MDL_NORENDER, false );
            recomputecamera();
        }
    }
}

