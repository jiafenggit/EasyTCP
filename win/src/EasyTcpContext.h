#ifndef EASYTCPCONTEXT_H
#define EASYTCPCONTEXT_H

#include <WinSock2.h>
#include <memory>
#include <functional>
#include "EasyTcpAutoBuffer.h"

namespace EasyTcp
{
    namespace Context
    {
        class IAction;

        typedef std::function<void(IAction*)> CALLBACK1;
        typedef std::function<void(IAction*, int)> CALLBACK2;

        class Context : public OVERLAPPED
        {
        public:
            Context(IAction* action);
            ~Context();

            IAction* action();

        private:
            IAction* m_action;
        };
        typedef std::shared_ptr<Context> SPTRContext;

        //action
        class IAction : public std::enable_shared_from_this<IAction>
        {
        public:
            IAction(const SOCKET sock);
            virtual ~IAction();
            Context* context();

            virtual bool invoke(int& err) = 0;
            virtual void update(unsigned int increase) = 0;
            virtual void error(int err) = 0;

        protected:
            SPTRContext m_context;
            const SOCKET m_socket;
        };
        typedef std::shared_ptr<IAction> SPTRIAction;

        //connect
        class ConnectAction : public IAction
        {
        public:
            ConnectAction(const SOCKET sock,
                const std::string &host, unsigned short port,
                CALLBACK1 connectedCallback = nullptr,
                CALLBACK2 connectFailedCallback = nullptr);

            bool invoke(int& err);
            void update(unsigned int increase);
            void error(int err);

        protected:
            std::string m_host;
            unsigned short m_port;
            CALLBACK1 m_connectedCallback;
            CALLBACK2 m_connectFailedCallback;
        };

        //disconnect
        class DisconnectAction : public IAction
        {
        public:
            DisconnectAction(const SOCKET sock,
                CALLBACK1 disconnectedCallback = NULL);

            bool invoke(int& err);
            void update(unsigned int increase);
            void error(int err);

        protected:
            CALLBACK1 m_disconnectedCallback;
        };


        //transmit
        class TransmitAction : public IAction
        {
        public:
            TransmitAction(const SOCKET sock, EasyTcp::AutoBuffer data,
                CALLBACK1 finishedCallback = nullptr,
                CALLBACK2 badCallback = nullptr);

            bool invoke(int& err);
            void update(unsigned int increase);
            void error(int err);

            EasyTcp::AutoBuffer data();

        protected:
            virtual bool transmit(int& err) = 0;

        protected:
            EasyTcp::AutoBuffer m_data;
            WSABUF m_wsaBuffer;
            CALLBACK1 m_finishedCallback;
            CALLBACK2 m_badCallback;
            unsigned int m_progress;
        };

        //send
        class SendAction : public TransmitAction
        {
        public:
            SendAction(const SOCKET sock, EasyTcp::AutoBuffer data,
                       CALLBACK1 finishedCallback = nullptr,
                       CALLBACK2 badCallback = nullptr);

        protected:
            bool transmit(int& err);
        };

        //receive
        class ReceiveAction : public TransmitAction
        {
        public:
            ReceiveAction(const SOCKET sock, EasyTcp::AutoBuffer data,
                          CALLBACK1 finishedCallback = nullptr,
                          CALLBACK2 badCallback = nullptr);

        protected:
            bool transmit(int& err);

        protected:
            unsigned long m_flags;
        };
    }
}

#endif
