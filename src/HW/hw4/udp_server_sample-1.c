while (1)
{
	clnt_adr_sz = sizeof(clnt_adr);
	str_len = recvfrom(serv_sock, &req_packet, sizeof(REQ_PACKET), 0,
					   (struct sockaddr *)&clnt_adr, &clnt_adr_sz);
	printf("[Server] Rx cmd=%d, ch=%c\n", req_packet.cmd, req_packet.ch);

	if (빈 공간이 없으면 != 1)
	{
		printf("No empty space. Exit this program.\n");

		// GAME_END 패킷 전송
		res_packet.cmd = GAME_END;
		res_packet.result = 0;

		sendto(serv_sock, &res_packet, sizeof(RES_PACKET), 0,
			   (struct sockaddr *)&clnt_adr, clnt_adr_sz);

		break;
	}

	if (req_packet.cmd == GAME_REQ)
	{
		// 기능 구현

		res_packet.cmd = GAME_RES;
		res_packet.result = count;
		
		sendto(serv_sock, &res_packet, sizeof(RES_PACKET), 0,
			   (struct sockaddr *)&clnt_adr, clnt_adr_sz);
		sleep(1);
	}
	else 
	{
		printf("Invalid cmd: %d\n", req_packet.cmd);
	}
}
close(sock);

