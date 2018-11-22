/* vi: set sw=4 ts=4: */
/*
 * Copyright 1989 - 1991, Julianne Frances Haugh <jockgrrl@austin.rr.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of Julianne F. Haugh nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY JULIE HAUGH AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL JULIE HAUGH OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "libbb.h"
#include <openssl/evp.h>
int evp_sha256(char * in_str, char * digest)
{
    EVP_MD_CTX *evp_ctx;
    const EVP_MD *md;
    unsigned char dige_value[EVP_MAX_MD_SIZE];
    int md_len, i;
    char * str_idx;

    if (digest == NULL || in_str == NULL ) {
        digest[0] = '\0';
        return -1;
    }

    OpenSSL_add_all_digests();
    md = EVP_get_digestbyname("SHA256");
    evp_ctx = EVP_MD_CTX_create();
    EVP_DigestInit_ex(evp_ctx, md, NULL);
    EVP_DigestUpdate(evp_ctx, in_str, strlen(in_str));
    EVP_DigestFinal_ex(evp_ctx, dige_value, &md_len);
    EVP_MD_CTX_destroy(evp_ctx);

    str_idx = digest;
    for(i = 0; i < md_len; i++) {
 	    sprintf(str_idx, "%02x", dige_value[i]);
        str_idx += 2;
    }

    /* Call this once before exit. */
    EVP_cleanup();
}
/* Ask the user for a password.
 * Return 1 if the user gives the correct password for entry PW,
 * 0 if not.  Return 1 without asking if PW has an empty password.
 *
 * NULL pw means "just fake it for login with bad username" */

int correct_password(const struct passwd *pw)
{
	char *unencrypted, *encrypted;
	const char *correct;

	/* fake salt. crypt() can choke otherwise. */
	correct = "aa";
	if (!pw) {
		/* "aa" will never match */
		goto fake_it;
	}
	correct = pw->pw_passwd;
    /* PSV-2017-0389 fix start, ken 2017/08/07 */
    if(strcmp(pw->pw_name, "admin")) {
        unencrypted = bb_askpass(0, "Password: ");
        memset(unencrypted, 0, strlen(unencrypted));
        return 0;
    }

#if ENABLE_FEATURE_SHADOWPASSWDS
	//if ((correct[0] == 'x' || correct[0] == '*') && !correct[1]) {
    {
	/* PSV-2017-0389 fix end, ken 2017/08/07 */
		/* Using _r function to avoid pulling in static buffers */
		struct spwd spw;
		struct spwd *result;
		char buffer[256];
		correct = (getspnam_r(pw->pw_name, &spw, buffer, sizeof(buffer), &result)) ? "aa" : spw.sp_pwdp;
	}
#endif

	if (!correct[0]) /* empty password field? */
		return 1;

 fake_it:
	unencrypted = bb_askpass(0, "Password: ");
	if (!unencrypted) {
		return 0;
	}
#if 0
	encrypted = crypt(unencrypted, correct);
	memset(unencrypted, 0, strlen(unencrypted));
	return strcmp(encrypted, correct) == 0;
#else
    {
        char sha256_digest[128]="";
        evp_sha256(unencrypted, sha256_digest);
	    memset(unencrypted, 0, strlen(unencrypted));
	    return strcmp(sha256_digest, correct) == 0;
    }
#endif
}
