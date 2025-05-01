while (1)
{
	// recvfrom()
	str_len = recvfrom();

	printf("[Client] Rx cmd=%d, result=%d\n", res_packet.cmd, res_packet.result);

	if (res_packet.cmd == GAME_RES)
	{
		// 기능 구현 
		req_packet.cmd = GAME_REQ;
		req_packet.ch = get_random_alphabet();

		sendto(sock, &req_packet, sizeof(REQ_PACKET), 0,
			   (struct sockaddr *)&serv_adr, sizeof(serv_adr));

	}
	else if (res_packet.cmd == GAME_END)
	{
		printf("No empty space. Exit this program.\n");
		break;
	}
	else 
	{
		printf("Invalid cmd: %d\n", res_packet.cmd);
	}
}
close(sock);