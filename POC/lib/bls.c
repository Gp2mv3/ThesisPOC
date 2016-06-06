//pairing-based signatures library
#include <stdlib.h> //for pbc_malloc, pbc_free
#include <string.h> //for memcmp()

#include "inc/bls.h"
#include "inc/hash.h"

void bls_sign(unsigned char *sig, unsigned int hashlen, unsigned char *hash, bls_private_key_t sk)
{
	element_t h;

	element_init_G1(h, sk->param->pairing);
	element_from_hash(h, hash, hashlen);
	element_pow_zn(h, h, sk->x);
	element_to_bytes_x_only(sig, h);

	element_clear(h);
}

int bls_verify(unsigned char *sig, unsigned int hashlen, unsigned char *hash, bls_public_key_t pk)
{
	//have to mess with internals since we are only given the x-coord
	element_t hx;
	element_t h;
	int res;

	pairing_ptr pairing = pk->param->pairing;

	element_init_G1(h, pairing);
	element_from_hash(h, hash, hashlen);
	element_init_G1(hx, pairing);
	element_from_bytes_x_only(hx, sig);

	res = is_almost_coddh(h, hx, pk->param->g, pk->gx, pk->param->pairing);

	element_clear(hx);
	element_clear(h);
	return res;
}

void bls_gen_sys_param(bls_sys_param_t param, pairing_t pairing)
{
	param->pairing = pairing;
	element_init_G2(param->g, pairing);
	element_random(param->g);
	param->signature_length = pairing_length_in_bytes_x_only_G1(pairing);
}

void bls_clear_sys_param(bls_sys_param_t param)
{
	element_clear(param->g);
}

void bls_gen(bls_public_key_t pk, bls_private_key_t sk, bls_sys_param_t param)
{
	pk->param = sk->param = param;
	element_init_G2(pk->gx, param->pairing);
	element_init_Zr(sk->x, param->pairing);
	element_random(sk->x);
	element_pow_zn(pk->gx, param->g, sk->x);
}

void bls_clear_public_key(bls_public_key_t pk)
{
	element_clear(pk->gx);
}

void bls_clear_private_key(bls_private_key_t sk)
{
	element_clear(sk->x);
}
