#ifndef CONSTANT_H
#define CONSTANT_H

#define NAME_HISTORY_FEEDBACK "HistoryFeedback"
#define NAME_PROBLEM_FEEDBACK "ProblemFeedback"
#define NAME_SELF_SERVICE "SelfService"
#define NAME_SERVICE_ONLINE "ServiceOnline"

enum InternalMode { InZentao = 0, OutZentao = 1, ProjectMode = 2, OpenkylinMode = 3, DefaultMode = 4 };
enum BeFrom { FromKOM = 0, FromDomain };

#endif // CONSTANT_H
