<?php

class VarnishAdmin {
	public function __construct(array $opt = []);

	public function connect(): bool;

	public function auth(): bool;

	public function getParams(): array;

	public function setParam(string $name, string $value): int;

	public function stop(): int;

	public function start(): int;

	public function banUrl(string $regex): int;

	public function ban(string $regex): int;

	public function isRunning(): bool;

	public function getPanic(): string;

	public function clearPanic(): int;

	public function setHost(string $host): void;

	public function setIdent(string $ident): void;

	public function setSecret(string $secret): void;

	public function setTimeout(int $timeout): void;

	public function setPort(int $port): void;

	public function setCompat(int $compat): void;

	public function getVclList(): array;

	public function vclUse(string $name): bool;

	public function disconnect(): bool;
}

class VarnishStat {
	public function __construct(array $opt = []);

	public function getSnapshot(): array;
}

class VarnishLog {
	public function __construct(array $opt = []);

	public function getLine(): array;

	public function getTagName(int $ind): string;
}
