//see Boneh, Boyen, "Short Group Signatures"
#include <pbc/pbc.h>
#include "inc/bb.h"
#include "inc/hash.h"



void bb_gen_sys_param(bb_sys_param_t param, pairing_t pairing)
{
	param->pairing = pairing;
	//signature is a point (only need one coordinate) and an element of Z_r
	param->signature_length = pairing_length_in_bytes_x_only_G1(pairing)
	+ pairing_length_in_bytes_Zr(pairing);
}

void bb_gen(bb_public_key_t pk, bb_private_key_t sk, bb_sys_param_t param)
{
	pairing_ptr pairing = param->pairing;
	pk->param = sk->param = param;

	element_init(sk->x, pairing->Zr);
	element_init(sk->y, pairing->Zr);
	element_random(sk->x);
	element_random(sk->y);
	element_init(pk->g1, param->pairing->G1);
	element_init(pk->g2, param->pairing->G2);
	element_init(pk->z, param->pairing->GT);
	element_random(pk->g2);
	element_random(pk->g1);
	element_init(pk->u, param->pairing->G2);
	element_init(pk->v, param->pairing->G2);
	element_pow_zn(pk->u, pk->g2, sk->x);
	element_pow_zn(pk->v, pk->g2, sk->y);
	element_pairing(pk->z, pk->g1, pk->g2);
}

void bb_sign(unsigned char *sig, unsigned int hashlen, unsigned char *hash, bb_public_key_t pk, bb_private_key_t sk)
{
	int len;
	element_t sigma;
	element_t r, z, m;
	bb_sys_param_ptr param = pk->param;
	pairing_ptr pairing = param->pairing;

	element_init(r, pairing->Zr);
	element_init(z, pairing->Zr);
	element_init(m, pairing->Zr);

	element_random(r);
	element_from_hash(m, hash, hashlen);
	element_mul(z, sk->y, r);
	element_add(z, z, sk->x);
	element_add(z, z, m);
	element_invert(z, z);
	element_init(sigma, pairing->G1);
	element_pow_zn(sigma, pk->g1, z);

	len = element_to_bytes_x_only(sig, sigma);
	element_to_bytes(&sig[len], r);

	element_clear(sigma);
	element_clear(r);
	element_clear(z);
	element_clear(m);
}

int bb_verify(unsigned char *sig, unsigned int hashlen, unsigned char *hash, bb_public_key_t pk)
{
	element_t sigma, r;
	element_t m;
	element_t t0, t1, t2;
	int res;
	int len;
	pairing_ptr pairing = pk->param->pairing;

	element_init(m, pairing->Zr);

	element_from_hash(m, hash, hashlen);

	element_init(sigma, pairing->G1);
	len = element_from_bytes_x_only(sigma, sig);

	element_init(r, pairing->Zr);
	element_from_bytes(r, sig + len);

	element_init(t0, pairing->G2);
	element_init(t1, pairing->G2);
	element_init(t2, pairing->GT);

	element_pow_zn(t0, pk->g2, m);
	element_pow_zn(t1, pk->v, r);
	element_mul(t0, t0, t1);
	element_mul(t0, t0, pk->u);
	element_pairing(t2, sigma, t0);
	if (!element_cmp(t2, pk->z)) {
		res = 1;
	} else {
		element_mul(t2, t2, pk->z);
		res = element_is1(t2);
	}

	element_clear(t0);
	element_clear(t1);
	element_clear(t2);
	element_clear(m);
	element_clear(sigma);
	element_clear(r);
	return res;
}

void bb_free_pk(bb_public_key_t pk)
{
	element_clear(pk->g1);
	element_clear(pk->g2);
	element_clear(pk->u);
	element_clear(pk->v);
	element_clear(pk->z);
}

void bb_free_sk(bb_private_key_t sk)
{
	element_clear(sk->x);
	element_clear(sk->y);
}
