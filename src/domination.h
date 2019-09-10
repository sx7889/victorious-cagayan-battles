#ifdef SERVMODE
VAR(domitkpenalty, 0, 1, 1);

struct domiservmode : servmode
#else
struct domiclientmode : clientmode
#endif
{
    static const int CAPTURERADIUS = 30;
    static const int FLAGLIMIT = 10;
    static const int RESPAWNSECS = 5;

    struct capturepoint
    {
        int id, version;
        vec spawnloc;
        int team, owntime;
        int claimteam;
#ifdef SERVMODE
        int claimtime;
#else
        float spawnangle;
        vec interploc;
        float interpangle;
        int interptime;
        gameent *capturer;
#endif

        capturepoint() : id(-1) { reset(); }

        void reset()
        {
            version = 0;
            spawnloc = vec(0, 0, 0);
#ifdef SERVMODE
            owntime = 0;
            claimtime = 0;
#else
            spawnangle = 0;
            interploc = vec(0, 0, 0);
            interpangle = 0;
            interptime = 0;
            capturer = 0;
#endif
            team = 0;
            claimteam = 0;
            owntime = 0;
        }

#ifndef SERVMODE
        vec pos() const
        {
            return spawnloc;
        }
#endif
    };

    vector<capturepoint> capturepoints;
    int scores[MAXTEAMS];

    void resetcapturepoints()
    {
        capturepoints.shrink(0);
        loopk(MAXTEAMS) scores[k] = 0;
    }

    bool addcapturepoint( int i, const vec &o )
    {
        while(capturepoints.length()<=i) capturepoints.add();
        capturepoint &f = capturepoints[i];
        f.id = i;
        f.reset();
        f.spawnloc = o;
        return true;
    }

    int totalscore(int team)
    {
        return validteam(team) ? scores[team-1] : 0;
    }

    int setscore(int team, int score)
    {
        if(validteam(team)) return scores[team-1] = score;
        return 0;
    }

    int addscore(int team, int score)
    {
        if(validteam(team)) return scores[team-1] += score;
        return 0;
    }

    bool hidefrags() { return true; }

    int getteamscore(int team)
    {
        return totalscore(team);
    }

    void getteamscores(vector<teamscore> &tscores)
    {
        loopk(MAXTEAMS) if(scores[k]) tscores.add(teamscore(k+1, scores[k]));
    }
    
#ifdef SERVMODE
    static const int RESETFLAGTIME = 10000;

    bool notgotflags;

    domiservmode() : notgotflags(false) {}

    void reset(bool empty)
    {
        resetcapturepoints();
        notgotflags = !empty;
    }

    void cleanup()
    {
        reset(false);
    }

    void setup()
    {
        reset(false);
        if(notgotitems || ments.empty()) return;
        loopv(ments)
        {
            entity &e = ments[i];
            if(e.type != CAPTUREPOINT) continue;
            if( !addcapturepoint( capturepoints.length(), e.o ) ) {
                break;
            }
        }
        notgotflags = false;
    }

    void newmap()
    {
        reset(true);
    }

    void leavegame(clientinfo *ci, bool disconnecting = false)
    {
    }

    void died(clientinfo *ci, clientinfo *actor)
    {
    }

    bool canspawn(clientinfo *ci, bool connecting)
    {
        return connecting || !ci->state.lastdeath || gamemillis+curtime-ci->state.lastdeath >= RESPAWNSECS*1000;
    }

    bool canchangeteam(clientinfo *ci, int oldteam, int newteam)
    {
        return true;
    }

    void changeteam(clientinfo *ci, int oldteam, int newteam)
    {
    }

    void scoreflag(clientinfo *ci, int goal, int relay = -1)
    {
        ci->state.flags++;
        int team = ci->team, score = addscore(team, 1);
        
        sendf(-1, 1, "ri9", N_SCOREFLAG, ci->clientnum, relay, relay >= 0 ? ++capturepoints[relay].version : -1, goal, ++capturepoints[goal].version, team, score, ci->state.flags);
        if(score >= FLAGLIMIT) startintermission();
    }

    void takeflag(clientinfo *ci, int i, int version)
    {
        if( notgotflags ) return;
        capturepoint &f = capturepoints[i];
        if(f.team!=ci->team)
        {
            f.claimteam = ci->team;
            f.claimtime = lastmillis;
            sendf( -1, 1, "ri5", N_STARTCAPTURE, ci->clientnum, i, ++f.version, f.claimteam );
        }
        else
        {
            //loopvj(capturepoints) if(capturepoints[j].owner==ci->clientnum) { scoreflag(ci, i, j); break; }
        }
    }

    void update()
    {
        bool scoresdirty = false;
        if(gamemillis>=gamelimit || notgotflags || capturepoints.empty() ) return;

        int total = capturepoints.length();
        int cpPerc = 100/total;

        loopk(MAXTEAMS) scores[k] = 0;

        loopv(capturepoints)
        {
            capturepoint &cp = capturepoints[i];
            const vec &loc = cp.spawnloc;
             
            if( cp.claimteam != cp.team ) {
                int gf = 0;
                int ga = 0;
                loopvj(clients) {
                    clientinfo *ci = clients[j];
                    vec op = ci->state.o;
                    if( ci->state.state==CS_ALIVE
                    && op.dist(loc) < CAPTURERADIUS ) {
                        if( cp.claimteam == ci->team ) {
                            ++ga;
                        }
                        else {
                            ++gf;
                        }
                    }
                }

                if( gf || !ga ) {
                    cp.claimteam = cp.team;
                    cp.claimtime = 0;
                    sendf( -1, 1, "ri4", N_CANCELCAPTURE, i, ++cp.version, cp.team );
                    scoresdirty = true;
                }
            }

            if( cp.team != cp.claimteam
            && lastmillis - cp.claimtime > 2000 ) {
                cp.team = cp.claimteam;
                if( capturereward ) {
                    loopvj(clients) {
                        clientinfo *ci = clients[j];
                        vec op = ci->state.o;
                        if( ci->state.state==CS_ALIVE
                        && ci->team == cp.team
                        && op.dist(loc) < CAPTURERADIUS ) {
                            servstate &gs = ci->state;
                            gs.health = gs.maxhealth;
                            sendf( -1, 1, "ri7", N_CLIENTUPDATE, ci->clientnum, gs.lifesequence, gs.health, gs.maxhealth, ci->cloakmode, gs.grabentity );
                        }
                    }
                }
                sendf( -1, 1, "ri3", N_CAPTURED, i, cp.claimteam );
                scoresdirty = true;
            }

            if( cp.team != 0 ) {
                scores[ cp.team-1 ] += cpPerc;
            }
        }

        if( scoresdirty ) {
            sendf( -1, 1, "ri3", N_DOMINATIONSCORES, scores[0], scores[1] );
        }
    }

    void initclient(clientinfo *ci, packetbuf &p, bool connecting)
    {
        putint(p, N_INITDOMINATION);
        loopk(2) putint(p, scores[k]);
        putint(p, capturepoints.length());
        loopv(capturepoints)
        {
            capturepoint &f = capturepoints[i];
            putint(p, f.version);
            putint(p, f.team);
            putint(p, f.claimteam);
        }
    }

    bool parseservermessage( int type, packetbuf &p, server::clientinfo *ci, server::clientinfo *cq, server::servmode *smode ) {
        domiservmode *domimode = static_cast< domiservmode* >( smode );
        switch( type ) {
            case N_STARTCAPTURE: {
                int flag = getint(p), version = getint(p);
                if((ci->state.state!=CS_SPECTATOR || ci->local || ci->privilege) && cq ) domimode->takeflag( cq, flag, version );
                return true;
            }
        }

        return false;
    }
};
#else
    #define FLAGCENTER 3.5f
    #define FLAGFLOAT 7

    void preload()
    {
        for(int i = S_FLAGPICKUP; i <= S_FLAGFAIL; i++) preloadsound(i);
    }

    void drawblip(gameent *d, float x, float y, float s, const vec &pos, bool flagblip)
    {
        float scale = calcradarscale();
        vec dir = d->o;
        dir.sub(pos).div(scale);
        float size = flagblip ? 0.1f : 0.05f,
              xoffset = flagblip ? -2*(3/32.0f)*size : -size,
              yoffset = flagblip ? -2*(1 - 3/32.0f)*size : -size,
              dist = dir.magnitude2(), maxdist = 1 - 0.05f - 0.05f;
        if(dist >= maxdist) dir.mul(maxdist/dist);
        dir.rotate_around_z(camera1->yaw*-RAD);
        drawradar(x + s*0.5f*(1.0f + dir.x + xoffset), y + s*0.5f*(1.0f + dir.y + yoffset), size*s);
    }

    void drawblip(gameent *d, float x, float y, float s, int i, bool flagblip)
    {
        capturepoint &f = capturepoints[i];
        setbliptex(f.team, flagblip ? "_flag" : "");
        drawblip(d, x, y, s, f.spawnloc, flagblip);
    }

    float clipconsole(float w, float h)
    {
        return (h*(1 + 1 + 10))/(4*10);
    }

    void drawhud(gameent *d, int w, int h)
    {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        int s = 1800/4, x = 1800*w/h - s - s/10, y = 1800*0.9f - (s + 2*0.04f);
        gle::colorf(1, 1, 1, minimapalpha);
        if(minimapalpha >= 1) glDisable(GL_BLEND);
        bindminimap();
        drawminimap(d, x, y, s);
        if(minimapalpha >= 1) glEnable(GL_BLEND);
        gle::colorf(1, 1, 1);
        float margin = 0.04f, roffset = s*margin, rsize = s + 2*roffset;
        setradartex();
        drawradar(x - roffset, y - roffset, rsize);
        settexture("media/interface/radar/compass.png", 3);
        pushhudmatrix();
        hudmatrix.translate(x - roffset + 0.5f*rsize, y - roffset + 0.5f*rsize, 0);
        hudmatrix.rotate_around_z((camera1->yaw + 180)*-RAD);
        flushhudmatrix();
        drawradar(-0.5f*rsize, -0.5f*rsize, rsize);
        pophudmatrix();
        drawplayerblip(d, x, y, s, 1.5f);
        loopv(capturepoints)
        {
            capturepoint &f = capturepoints[i];
            drawblip(d, x, y, s, i, true);
        }
        drawteammates(d, x, y, s);
    }

    void removeplayer(gameent *d)
    {
    }

    vec interpflagpos(capturepoint &f, float &angle)
    {
        vec pos = f.spawnloc;
        if(pos.x < 0) return pos;
        pos.addz(FLAGCENTER);
        if(f.interptime && f.interploc.x >= 0)
        {
            float t = min((lastmillis - f.interptime)/500.0f, 1.0f);
            pos.lerp(f.interploc, pos, t);
            angle += (1-t)*(f.interpangle - angle);
        }
        return pos;
    }

    vec interpflagpos(capturepoint &f) { float angle; return interpflagpos(f, angle); }

    void rendergame()
    {
        loopv(capturepoints)
        {
            capturepoint &f = capturepoints[i];
            const char *name = 
            f.team == 0 ? models[ MODEL_WHITE_FLAG ].name :
            models[ teams[ f.team ].worldflag ].name;
            float angle;
            vec pos = interpflagpos(f, angle);
            rendermodel(name, ANIM_MAPMODEL|ANIM_LOOP,
                        pos, angle, 0, 0,
                        MDL_CULL_VFC | MDL_CULL_OCCLUDED | MDL_CULL_DIST);
        }
    }

    void setup()
    {
        resetcapturepoints();
        loopv(entities::ents)
        {
            extentity *e = entities::ents[i];
            if(e->type!=CAPTUREPOINT) continue;
            int index = capturepoints.length();
            if( !addcapturepoint( index, e->o ) ) {
                continue;
            }
            capturepoints[index].spawnangle = e->attr1;
        }
    }

    void parseflags(ucharbuf &p, bool commit)
    {
        loopk(2)
        {
            int score = getint(p);
            if(commit) scores[k] = score;
        }
        int numflags = getint(p);
        loopi(numflags)
        {
            int version = getint(p), team = getint(p), claimteam = getint(p);
            if(commit && capturepoints.inrange(i))
            {
                capturepoint &f = capturepoints[i];
                f.version = version;
                f.team = team;
                f.claimteam = claimteam;
                f.interptime = 0;
            }
        }
    }

    const char *teamcolorflag(capturepoint &f)
    {
        return teamcolor("", "'s flag", f.team, "a flag");
    }

    void flagexplosion(int i, int team, const vec &loc)
    {
        int fcolor;
        vec color;
        if(team==1) { fcolor = 0x2020FF; color = vec(0.25f, 0.25f, 1); }
        else { fcolor = 0x802020; color = vec(1, 0.25f, 0.25f); }
        particle_fireball(loc, 30, PART_EXPLOSION, -1, fcolor, 4.8f);
        adddynlight(loc, 35, color, 900, 100);
        particle_splash(PART_SPARK, 150, 300, loc, 0.24f);
    }

    void flageffect(int i, int team, const vec &from, const vec &to)
    {
        if(from.x >= 0)
            flagexplosion(i, team, from);
        if(from==to) return;
        if(to.x >= 0)
            flagexplosion(i, team, to);
        if(from.x >= 0 && to.x >= 0)
            particle_flare(from, to, 600, PART_LIGHTNING, team==1 ? 0x2222FF : 0xFF2222, 1.0f);
    }

    void scoreflag(gameent *d, int relay, int relayversion, int goal, int goalversion, int team, int score, int dflags)
    {
        setscore(team, score);
        if(capturepoints.inrange(goal))
        {
            capturepoint &f = capturepoints[goal];
            f.version = goalversion;
            if(relay >= 0)
            {
                capturepoints[relay].version = relayversion;
                flageffect(goal, team, vec(f.spawnloc).addz(FLAGFLOAT+FLAGCENTER), vec(capturepoints[relay].spawnloc).addz(FLAGFLOAT+FLAGCENTER));
            }
            else flageffect(goal, team, interpflagpos(f), vec(f.spawnloc).addz(FLAGFLOAT+FLAGCENTER));
            f.interptime = 0;
        }
        if(d!=player1) particle_textcopy(d->abovehead(), tempformatstring("%d", score), PART_TEXT, 2000, 0x32FF64, 4.0f, -8);
        d->flags = dflags;
        conoutf(CON_GAMEINFO, "%s scored for %s", teamcolorname(d), teamcolor("team ", "", team, "a team"));
        playsound(team==player1->team ? S_FLAGSCORE : S_FLAGFAIL);

        if(score >= FLAGLIMIT) conoutf(CON_GAMEINFO, "%s captured %d flags", teamcolor("team ", "", team, "a team"), score);
    }

    void takeflag( gameent *d, int i, int version, int claimteam )
    {
        if(!capturepoints.inrange(i)) return;
        capturepoint &f = capturepoints[i];
        f.version = version;
        f.interploc = interpflagpos(f, f.interpangle);
        f.interptime = lastmillis;
        f.claimteam = claimteam;
        //if(f.droptime) conoutf(CON_GAMEINFO, "%s picked up %s", teamcolorname(d), teamcolorflag(f));
        //else conoutf(CON_GAMEINFO, "%s stole %s", teamcolorname(d), teamcolorflag(f));
    }

    void checkitems(gameent *d)
    {
        if(d->state!=CS_ALIVE) return;
        vec o = d->feetpos();
        loopv(capturepoints)
        {
            capturepoint &f = capturepoints[i];

            const vec &loc = f.spawnloc;
            if( o.dist(loc) < CAPTURERADIUS )
            {
                bool canbecapture = true;
                loopv(players)
                {
                    gameent *p = players[i];
                    vec op = p->feetpos();
                    if( p->state == CS_ALIVE
                    && f.team == p->team
                    && op.dist(loc) < CAPTURERADIUS ) {
                        canbecapture = false;
                        break;
                    }
                }

                if( !canbecapture ) {
                    continue;
                }

                if( f.claimteam != d->team ) {
                    addmsg(N_STARTCAPTURE, "rcii", d, i, f.version);
                    f.claimteam = d->team;
                }
            }
       }
    }

    void respawned(gameent *d)
    {
        vec o = d->feetpos();
        loopv(capturepoints)
        {
            capturepoint &f = capturepoints[i];
       }
    }

    int respawnwait(gameent *d)
    {
        return max(0, RESPAWNSECS-(lastmillis-d->lastpain)/1000);
    }

    bool aihomerun(gameent *d, ai::aistate &b)
    {
        vec pos = d->feetpos();
        loopk(2)
        {
            int goal = -1;
            loopv(capturepoints)
            {
                capturepoint &g = capturepoints[i];
                if(g.team == d->team &&
                    (!capturepoints.inrange(goal) || g.pos().squaredist(pos) < capturepoints[goal].pos().squaredist(pos)))
                {
                    goal = i;
                }
            }
            if(capturepoints.inrange(goal) && ai::makeroute(d, b, capturepoints[goal].pos()))
            {
                d->ai->switchstate(b, ai::AI_S_PURSUE, ai::AI_T_AFFINITY, goal);
                return true;
            }
        }
        if(b.type == ai::AI_S_INTEREST && b.targtype == ai::AI_T_NODE) return true; // we already did this..
        if(randomnode(d, b, ai::SIGHTMIN, 1e16f))
        {
            d->ai->switchstate(b, ai::AI_S_INTEREST, ai::AI_T_NODE, d->ai->route[0]);
            return true;
        }
        return false;
    }

    bool aicheck(gameent *d, ai::aistate &b)
    {
        vec p = d->feetpos();
        int cpcapture = -1, cpdef = -1;
        float cpcapturedist = 9999999.0f, cpdefdist = 9999999.0f;
        loopv(capturepoints)
        {
            capturepoint &g = capturepoints[i];
            const vec &loc = g.spawnloc;
            float dist = p.dist(loc);
            if( g.team != d->team ) {
                if( dist < cpcapturedist ) {
                    cpcapturedist = dist;
                    cpcapture = i;
                }
            }
            else if( g.claimteam != d->team ) {
                if( dist < cpdefdist ) {
                    cpdefdist = dist;
                    cpdef = i;
                }
            }
        }
        if( !ai::badhealth(d) )
        {
            if( cpcapture != -1 ) {
                d->ai->switchstate( b, ai::AI_S_PURSUE, ai::AI_T_AFFINITY, cpcapture );
                return true;
            }
            else if( cpdef != -1 ) {
                d->ai->switchstate( b, ai::AI_S_PURSUE, ai::AI_T_AFFINITY, cpdef );
                return true;
            }
        }
        return false;
    }

    void aifind(gameent *d, ai::aistate &b, vector<ai::interest> &interests)
    {
        vec pos = d->feetpos();
        loopvj(capturepoints)
        {
            capturepoint &f = capturepoints[j];

            if( f.team != d->team ) {
                if( f.claimteam == d->team ) { // defend the flag
                    conoutf( "defend" );
                    ai::interest &n = interests.add();
                    n.state = ai::AI_S_DEFEND;
                    n.node = ai::closestwaypoint(f.pos(), ai::SIGHTMIN, true);
                    n.target = j;
                    n.targtype = ai::AI_T_AFFINITY;
                    n.score = pos.squaredist(f.pos())/100.f;
                }
                else { // attack the flag
                    conoutf( "attack" );
                    ai::interest &n = interests.add();
                    n.state = ai::AI_S_PURSUE;
                    n.node = ai::closestwaypoint(f.pos(), ai::SIGHTMIN, true);
                    n.target = j;
                    n.targtype = ai::AI_T_AFFINITY;
                    n.score = pos.squaredist(f.pos());
                }
            }
        }
    }

    bool aidefend(gameent *d, ai::aistate &b)
    {
        if(capturepoints.inrange(b.target))
        {
            capturepoint &f = capturepoints[b.target];
            if( f.team == d->team
            && f.claimteam != f.team ) {
                int walk = 2;
                return ai::defend(d, b, f.pos(), float(CAPTURERADIUS*2), float(CAPTURERADIUS*(2+(walk*2))), walk);
            }
        }
        return false;
    }

    bool aipursue(gameent *d, ai::aistate &b)
    {
        if(capturepoints.inrange(b.target))
        {
            capturepoint &f = capturepoints[b.target];
            return ai::makeroute(d, b, f.pos());
        }
        return false;
    }
    
    bool parseclientmessage( int type, ucharbuf &p, clientmode *m ) {
        domiclientmode *domimode = static_cast< domiclientmode* >( m );
        switch( type ) {
            case N_INITDOMINATION:
            {
                domimode->parseflags(p, m_domination);
                return true;
            }

            case N_STARTCAPTURE:
            {
                int ocn = getint(p), flag = getint(p), version = getint(p), claimteam = getint(p);
                gameent *o = ocn==player1->clientnum ? player1 : newclient(ocn);
                if(o && m_domination) domimode->takeflag( o, flag, version, claimteam );
                return true;
            }

            case N_CANCELCAPTURE: {
                int pointindex = getint(p), version = getint(p), claimteam = getint(p);
                if( m_domination ) {
                    if(capturepoints.inrange(pointindex))
                    {
                        capturepoint &f = capturepoints[pointindex];
                        f.claimteam = claimteam;
                    }
                }
                return true;
            }

            case N_CAPTURED:
            {
                int flag = getint(p), claimteam = getint(p);
                if(capturepoints.inrange(flag))
                {
                    capturepoint &f = capturepoints[flag];
                    f.team = claimteam;
                    playsound( S_FLAGPICKUP );
                }
                return true;
            }

            case N_DOMINATIONSCORES: {
                int score1 = getint(p), score2 = getint(p);
                setscore( 1, score1 );
                setscore( 2, score2 );
                return true;
            }
        }

        return false;
    }
};

#endif

