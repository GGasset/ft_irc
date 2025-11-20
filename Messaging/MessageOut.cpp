#include "Message.hpp"

void	*MessageOut::get_msg() {
	return (static_cast<void*>(msg));
}

void	MessageOut::setTarget(std::unique_ptr<MessageTarget> *target) {
	this->target = target;
}