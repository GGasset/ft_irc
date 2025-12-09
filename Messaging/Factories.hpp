#pragma once

#include "MessageOut.hpp"


class NumericReplyFactory {
    Server  &server;

public:

    static NumericReply *create(ReplyCode code, Server &serv, Param *param);
    static NumericReply *create_and_target(ReplyCode code, Server &serv, Param *param, std::vector<size_t> ids, char t);
    static NumericReply *create_and_target(ReplyCode code, Server &serv, Param *param, size_t id, char t);

    NumericReplyFactory(Server &server): server(server) {}

    /* WELCOME */
    static RplWelcome        *makeRplWelcome(Server &serv, UserParam* p) { return new RplWelcome(serv, p); }
    static RplYourHost       *makeRplYourHost(Server &serv, UserParam* p) { return new RplYourHost(serv, p); }
    static RplCreated        *makeRplCreated(Server &serv, UserParam* p) { return new RplCreated(serv, p); }

    /* NICK */
    static ErrErroneousNickname *makeErrErroneusNickname(Server &s, NickParam* p) { return new ErrErroneousNickname(s, p); }
    static ErrNoNicknamegiven   *makeErrNoNicknamegiven(Server &s, NickParam* p) { return new ErrNoNicknamegiven(s, p); }
    static ErrNicknameInUse     *makeErrNicknameInUse(Server &s, NickParam* p) { return new ErrNicknameInUse(s, p); }
    static ErrUnavailResource   *makeErrUnavailResource(Server &s, NickParam* p) { return new ErrUnavailResource(s, p); }
    static ErrRestricted        *makeErrRestricted(Server &s, NickParam* p) { return new ErrRestricted(s, p); }

    /* GENERIC */
    static ErrNeedMoreParams    *makeErrNeedMoreParams(Server &s, Param *p) { return new ErrNeedMoreParams(s, p->command()); }
	static ErrUnknownCommand		*makeErrUnknownCommand(Server &serv) {return new ErrUnknownCommand(serv);}


    /* PASS / USER */
    static ErrAlredyRegistered  *makeErrAlredyRegistered(Server &s, Param *p) {
        // válido para USER y PASS
        if (dynamic_cast<UserParam*>(p))
            return new ErrAlredyRegistered(s, static_cast<UserParam*>(p));
        if (dynamic_cast<PassParam*>(p))
            return new ErrAlredyRegistered(s, static_cast<PassParam*>(p));
        return NULL;
    }

    /* PING / PONG */
    static ErrNoOrigin       *makeErrNoOrigin(Server &s, PingPongParam *p) { return new ErrNoOrigin(s, p); }
    static ErrNoSuchServer   *makeErrNoSuchServer(Server &s, PingPongParam *p) { return new ErrNoSuchServer(s, p); }

    /* PART */
    static ErrNotOnChannel   *makeErrNotOnChannel(Server &s, PartParam *p) { return new ErrNotOnChannel(s, p); }

    /* PRIVMSG */
    static ErrNoRecipient      *makeErrNoRecipient(Server &s, PrivMsgParam *p) { return new ErrNoRecipient(s, p); }
    static ErrNoTextToSend     *makeErrNoTextToSend(Server &s, PrivMsgParam *p) { return new ErrNoTextToSend(s, p); }
    static ErrNoSuchNick       *makeErrNoSuchNick(Server &s, PrivMsgParam *p) { return new ErrNoSuchNick(s, p); }
    static ErrCannotSendToChan *makeErrCannotSendToChan(Server &s, PrivMsgParam *p) { return new ErrCannotSendToChan(s, p); }
    static ErrTooManyTargets   *makeErrTooManyTargets(Server &s, PrivMsgParam *p) { return new ErrTooManyTargets(s, p); }
    static ErrNoTopLevel       *makeErrNoTopLevel(Server &s, PrivMsgParam *p) { return new ErrNoTopLevel(s, p); }
    static ErrWildTopLevel     *makeErrWildTopLevel(Server &s, PrivMsgParam *p) { return new ErrWildTopLevel(s, p); }

    /* MODE */
    static RplChannelModeIs   *makeRplChannelModeIs(Server &s, ModeParam *p) { return new RplChannelModeIs(s, p); }
    static ErrUnknownMode     *makeErrUnknownMode(Server &s, ModeParam *p) { return new ErrUnknownMode(s, p); }
    static ErrUModeUnknownFlag*makeErrUModeUnknownFlag(Server &s, ModeParam *p) { return new ErrUModeUnknownFlag(s, p); }
    static ErrKeySet          *makeErrKeySet(Server &s, ModeParam *p) { return new ErrKeySet(s, p); }
    static ErrUserNotInChannel*makeErrUserNotInChannel(Server &s, ModeParam *p) { return new ErrUserNotInChannel(s, p); }

    /* INVITE */
    static RplInviting        *makeRplInviting(Server &s, InviteParam *p) { return new RplInviting(s, p); }
    static ErrUserOnChannel   *makeErrUserOnChannel(Server &s, InviteParam *p) { return new ErrUserOnChannel(s, p); }

    /* JOIN (ya hechos en mensaje anterior) */
    static RplNoTopic         *makeRplNoTopic(Server &s, JoinParam *p) { return new RplNoTopic(s, p); }
    static RplTopic           *makeRplTopic(Server &s, JoinParam *p) { return new RplTopic(s, p); }
    static ErrNoSuchChannel   *makeErrNoSuchChannel(Server &s, JoinParam *p) { return new ErrNoSuchChannel(s, p); }
    static ErrTooManyChannels *makeErrTooManyChannels(Server &s, JoinParam *p) { return new ErrTooManyChannels(s, p); }
    static ErrChannelIsFull   *makeErrChannelIsFull(Server &s, JoinParam *p) { return new ErrChannelIsFull(s, p); }
    static ErrInviteOnlyChan  *makeErrInviteOnlyChan(Server &s, JoinParam *p) { return new ErrInviteOnlyChan(s, p); }
    static ErrBannedFromChan  *makeErrBannedFromChan(Server &s, JoinParam *p) { return new ErrBannedFromChan(s, p); }
    static ErrBadChannelKey   *makeErrBadChannelKey(Server &s, JoinParam *p) { return new ErrBadChannelKey(s, p); }
    static ErrBadChanMask     *makeErrBadChanMask(Server &s, JoinParam *p) { return new ErrBadChanMask(s, p); }
    static ErrUnsupportedChanMode *makeErrUnsupportedChanMode(Server &s, JoinParam *p) { return new ErrUnsupportedChanMode(s, p); }
};


class ForwardedCommandFactory {
	Server	&server;
	
	public:
	ForwardedCommandFactory(Server &server): server(server) {}
	static ForwardedCommand	*makeNickForward(Server &serv, NickParam *param) {return new NickForwardedCommand(serv, param);}
	static ForwardedCommand	*makePingPongForward(Server &serv, PingPongParam *param) {return new PongForwardedCommand(serv, param);}
	static ForwardedCommand	*makeQuitForward(Server &serv, QuitParam *param) {return new QuitForwardedCommand(serv, param);}
	static ForwardedCommand	*makeJoinForward(Server &serv, JoinParam *param) {return new JoinForwardedCommand(serv, param);}
	static ForwardedCommand	*makePartForward(Server &serv, PartParam *param) {return new PartForwardedCommand(serv, param);}
	static ForwardedCommand	*makePrivMsgForward(Server &serv, PrivMsgParam *param) {return new PrivMsgForwardedCommand(serv, param);}
	static ForwardedCommand	*makeNoticeForward(Server &serv, NoticeParam *param) {return new NoticeForwardedCommand(serv, param);}
	static ForwardedCommand	*makeTopicForward(Server &serv, TopicParam *param) {return new TopicForwardedCommand(serv, param);}
	static ForwardedCommand	*makeInviteForward(Server &serv, InviteParam *param) {return new InviteForwardedCommand(serv, param);}
	static ForwardedCommand	*makeKickForward(Server &serv, KickParam *param) {return new KickForwardedCommand(serv, param);}
	static ForwardedCommand	*makeModeForward(Server &serv, ModeParam *param) {return new ModeForwardedCommand(serv, param);}
	static ForwardedCommand	*create(COMMAND cmd, Server &serv, Param *param);
};