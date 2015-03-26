/*
 * MessageIDSwitch.h
 *
 *  Created on: 2013年10月6日
 *      Author: space
 */

#ifndef MESSAGEIDSWITCH_H_
#define MESSAGEIDSWITCH_H_

switch ((MessageID)id) {

		case ServerChallenge_ID: {
			ServerChallenge scMessage;
			local_messagepack.msgUnpack<ServerChallenge>(scMessage,net_message.begin()+2,net_message.size()-2);
			MessageHandler(scMessage);
			break;
		}

		//case 1001:

		default:
			break;
		}


#endif /* MESSAGEIDSWITCH_H_ */
