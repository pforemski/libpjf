/*
 * This file is part of libasn
 * Copyright (C) 2005-2010 ASN Sp. z o.o.
 * Author: Pawel Foremski <pforemski@asn.pl>
 *
 * libasn is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 3 of the License, or (at your option);
 * any later version.
 *
 * libasn is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "lib.h"

const char *asn_ext2mime(const char *ext)
{
#define CHECK(e, m) if (streq(ext, #e)) return #m

	switch (ext[0]) {
	case '3':
		CHECK(3dm, x-world/x-3dmf);
		break;
	case 'a':
		CHECK(a, application/octet-stream);
		CHECK(aab, application/x-authorware-bin);
		CHECK(aam, application/x-authorware-map);
		CHECK(aas, application/x-authorware-seg);
		CHECK(abc, text/vnd.abc);
		CHECK(acgi, text/html);
		CHECK(afl, video/animaflex);
		CHECK(ai, application/postscript);
		CHECK(aif, audio/aiff);
		CHECK(aifc, audio/aiff);
		CHECK(aiff, audio/aiff);
		CHECK(aim, application/x-aim);
		CHECK(aip, text/x-audiosoft-intra);
		CHECK(ani, application/x-navi-animation);
		CHECK(aos, application/x-nokia-9000-communicator-add-on-software);
		CHECK(aps, application/mime);
		CHECK(arc, application/octet-stream);
		CHECK(arj, application/arj);
		CHECK(art, image/x-jg);
		CHECK(asf, video/x-ms-asf);
		CHECK(asm, text/x-asm);
		CHECK(asp, text/asp);
		CHECK(asx, application/x-mplayer2);
		CHECK(au, audio/basic);
		CHECK(au, audio/x-au);
		CHECK(avi, video/avi);
		CHECK(avs, video/avs-video);
		break;
	case 'b':
		CHECK(bcpio, application/x-bcpio);
		CHECK(bin, application/octet-stream);
		CHECK(bm, image/bmp);
		CHECK(bmp, image/bmp);
		CHECK(boo, application/book);
		CHECK(book, application/book);
		CHECK(boz, application/x-bzip2);
		CHECK(bsh, application/x-bsh);
		CHECK(bz, application/x-bzip);
		CHECK(bz2, application/x-bzip2);
		break;
	case 'c':
		CHECK(c, text/plain);
		CHECK(c++, text/x-c);
		CHECK(cat, application/vnd.ms-pki.seccat);
		CHECK(cc, text/plain);
		CHECK(ccad, application/clariscad);
		CHECK(cco, application/x-cocoa);
		CHECK(cdf, application/cdf);
		CHECK(cer, application/pkix-cert);
		CHECK(cha, application/x-chat);
		CHECK(chat, application/x-chat);
		CHECK(class, application/java);
		CHECK(com, application/octet-stream);
		CHECK(conf, text/plain);
		CHECK(cpio, application/x-cpio);
		CHECK(cpp, text/x-c);
		CHECK(cpt, application/mac-compactpro);
		CHECK(crl, application/pkcs-crl);
		CHECK(csh, application/x-csh);
		CHECK(css, application/x-pointplus);
		CHECK(cxx, text/plain);
		break;
	case 'd':
		CHECK(dcr, application/x-director);
		CHECK(deepv, application/x-deepv);
		CHECK(def, text/plain);
		CHECK(der, application/x-x509-ca-cert);
		CHECK(dif, video/x-dv);
		CHECK(dir, application/x-director);
		CHECK(dl, video/dl);
		CHECK(doc, application/msword);
		CHECK(dot, application/msword);
		CHECK(dp, application/commonground);
		CHECK(drw, application/drafting);
		CHECK(dump, application/octet-stream);
		CHECK(dv, video/x-dv);
		CHECK(dvi, application/x-dvi);
		CHECK(dwf, drawing/x-dwf (old));
		CHECK(dwg, application/acad);
		CHECK(dwg, image/x-dwg);
		CHECK(dxf, application/dxf);
		break;
	case 'e':
		CHECK(el, text/x-script.elisp);
		CHECK(elc, application/x-bytecode.elisp);
		CHECK(elc, application/x-elc);
		CHECK(env, application/x-envoy);
		CHECK(eps, application/postscript);
		CHECK(es, application/x-esrehber);
		CHECK(etx, text/x-setext);
		CHECK(evy, application/envoy);
		CHECK(evy, application/x-envoy);
		CHECK(exe, application/octet-stream);
		break;
	case 'f':
		CHECK(f, text/x-fortran);
		CHECK(f77, text/x-fortran);
		CHECK(f90, text/x-fortran);
		CHECK(fdf, application/vnd.fdf);
		CHECK(fif, image/fif);
		CHECK(fli, video/fli);
		CHECK(flo, image/florian);
		CHECK(flx, text/vnd.fmi.flexstor);
		CHECK(fmf, video/x-atomic3d-feature);
		CHECK(for, text/plain);
		CHECK(for, text/x-fortran);
		CHECK(fpx, image/vnd.fpx);
		CHECK(frl, application/freeloader);
		CHECK(funk, audio/make);
		break;
	case 'g':
		CHECK(g, text/plain);
		CHECK(g3, image/g3fax);
		CHECK(gif, image/gif);
		CHECK(gl, video/gl);
		CHECK(gsd, audio/x-gsm);
		CHECK(gsm, audio/x-gsm);
		CHECK(gsp, application/x-gsp);
		CHECK(gss, application/x-gss);
		CHECK(gtar, application/x-gtar);
		CHECK(gz, application/x-compressed);
		CHECK(gzip, application/x-gzip);
		break;
	case 'h':
		CHECK(h, text/plain);
		CHECK(hdf, application/x-hdf);
		CHECK(help, application/x-helpfile);
		CHECK(hgl, application/vnd.hp-hpgl);
		CHECK(hh, text/plain);
		CHECK(hlb, text/x-script);
		CHECK(hlp, application/hlp);
		CHECK(hpg, application/vnd.hp-hpgl);
		CHECK(hpgl, application/vnd.hp-hpgl);
		CHECK(hqx, application/binhex);
		CHECK(hta, application/hta);
		CHECK(htc, text/x-component);
		CHECK(htm, text/html);
		CHECK(html, text/html);
		CHECK(htmls, text/html);
		CHECK(htt, text/webviewhtml);
		CHECK(htx, text/html);
		break;
	case 'i':
		CHECK(ice, x-conference/x-cooltalk);
		CHECK(ico, image/x-icon);
		CHECK(idc, text/plain);
		CHECK(ief, image/ief);
		CHECK(iefs, image/ief);
		CHECK(iges, application/iges);
		CHECK(igs, application/iges);
		CHECK(ima, application/x-ima);
		CHECK(imap, application/x-httpd-imap);
		CHECK(inf, application/inf);
		CHECK(ins, application/x-internett-signup);
		CHECK(ip, application/x-ip2);
		CHECK(isu, video/x-isvideo);
		CHECK(it, audio/it);
		CHECK(iv, application/x-inventor);
		CHECK(ivr, i-world/i-vrml);
		CHECK(ivy, application/x-livescreen);
		break;
	case 'j':
		CHECK(jam, audio/x-jam);
		CHECK(jav, text/plain);
		CHECK(java, text/plain);
		CHECK(jcm, application/x-java-commerce);
		CHECK(jfif, image/jpeg);
		CHECK(jpe, image/jpeg);
		CHECK(jpeg, image/jpeg);
		CHECK(jpg, image/jpeg);
		CHECK(jps, image/x-jps);
		CHECK(js, application/x-javascript);
		CHECK(jut, image/jutvision);
		break;
	case 'k':
		CHECK(kar, audio/midi);
		CHECK(ksh, application/x-ksh);
		break;
	case 'l':
		CHECK(la, audio/nspaudio);
		CHECK(lam, audio/x-liveaudio);
		CHECK(latex, application/x-latex);
		CHECK(lha, application/lha);
		CHECK(lhx, application/octet-stream);
		CHECK(list, text/plain);
		CHECK(lma, audio/nspaudio);
		CHECK(log, text/plain);
		CHECK(lsp, application/x-lisp);
		CHECK(lst, text/plain);
		CHECK(lsx, text/x-la-asf);
		CHECK(ltx, application/x-latex);
		CHECK(lzh, application/octet-stream);
		break;
	case 'm':
		CHECK(m, text/plain);
		CHECK(m1v, video/mpeg);
		CHECK(m2a, audio/mpeg);
		CHECK(m2v, video/mpeg);
		CHECK(m3u, audio/x-mpequrl);
		CHECK(man, application/x-troff-man);
		CHECK(map, application/x-navimap);
		CHECK(mar, text/plain);
		CHECK(mbd, application/mbedlet);
		CHECK(mc$, application/x-magic-cap-package-1.0);
		CHECK(mcd, application/mcad);
		CHECK(mcf, text/mcf);
		CHECK(mcp, application/netmc);
		CHECK(me, application/x-troff-me);
		CHECK(mht, message/rfc822);
		CHECK(mhtml, message/rfc822);
		CHECK(mid, application/x-midi);
		CHECK(midi, application/x-midi);
		CHECK(mif, application/x-frame);
		CHECK(mime, message/rfc822);
		CHECK(mjf, audio/x-vnd.audioexplosion.mjuicemediafile);
		CHECK(mjpg, video/x-motion-jpeg);
		CHECK(mm, application/base64);
		CHECK(mme, application/base64);
		CHECK(mod, audio/mod);
		CHECK(moov, video/quicktime);
		CHECK(mov, video/quicktime);
		CHECK(movie, video/x-sgi-movie);
		CHECK(mp2, audio/mpeg);
		CHECK(mp3, audio/mpeg3);
		CHECK(mpa, audio/mpeg);
		CHECK(mpc, application/x-project);
		CHECK(mpe, video/mpeg);
		CHECK(mpeg, video/mpeg);
		CHECK(mpg, audio/mpeg);
		CHECK(mpga, audio/mpeg);
		CHECK(mpp, application/vnd.ms-project);
		CHECK(mpt, application/x-project);
		CHECK(mpv, application/x-project);
		CHECK(mpx, application/x-project);
		CHECK(mrc, application/marc);
		CHECK(ms, application/x-troff-ms);
		CHECK(mv, video/x-sgi-movie);
		CHECK(my, audio/make);
		CHECK(mzz, application/x-vnd.audioexplosion.mzz);
		break;
	case 'n':
		CHECK(nap, image/naplps);
		CHECK(naplps, image/naplps);
		CHECK(nc, application/x-netcdf);
		CHECK(ncm, application/vnd.nokia.configuration-message);
		CHECK(nif, image/x-niff);
		CHECK(niff, image/x-niff);
		CHECK(nix, application/x-mix-transfer);
		CHECK(nsc, application/x-conference);
		CHECK(nvd, application/x-navidoc);
		break;
	case 'o':
		CHECK(o, application/octet-stream);
		CHECK(oda, application/oda);
		CHECK(omc, application/x-omc);
		CHECK(omcd, application/x-omcdatamaker);
		CHECK(omcr, application/x-omcregerator);
		break;
	case 'p':
		CHECK(p, text/x-pascal);
		CHECK(p10, application/pkcs10);
		CHECK(p12, application/pkcs-12);
		CHECK(p7a, application/x-pkcs7-signature);
		CHECK(p7c, application/pkcs7-mime);
		CHECK(p7m, application/pkcs7-mime);
		CHECK(p7r, application/x-pkcs7-certreqresp);
		CHECK(p7s, application/pkcs7-signature);
		CHECK(part, application/pro_eng);
		CHECK(pas, text/pascal);
		CHECK(pbm, image/x-portable-bitmap);
		CHECK(pcl, application/vnd.hp-pcl);
		CHECK(pct, image/x-pict);
		CHECK(pcx, image/x-pcx);
		CHECK(pdb, chemical/x-pdb);
		CHECK(pdf, application/pdf);
		CHECK(pfunk, audio/make);
		CHECK(pgm, image/x-portable-graymap);
		CHECK(pic, image/pict);
		CHECK(pict, image/pict);
		CHECK(pkg, application/x-newton-compatible-pkg);
		CHECK(pko, application/vnd.ms-pki.pko);
		CHECK(pl, text/plain);
		CHECK(plx, application/x-pixclscript);
		CHECK(pm, image/x-xpixmap);
		CHECK(pm4, application/x-pagemaker);
		CHECK(pm5, application/x-pagemaker);
		CHECK(png, image/png);
		CHECK(pnm, application/x-portable-anymap);
		CHECK(pot, application/mspowerpoint);
		CHECK(pov, model/x-pov);
		CHECK(ppa, application/vnd.ms-powerpoint);
		CHECK(ppm, image/x-portable-pixmap);
		CHECK(pps, application/mspowerpoint);
		CHECK(ppt, application/mspowerpoint);
		CHECK(ppz, application/mspowerpoint);
		CHECK(pre, application/x-freelance);
		CHECK(prt, application/pro_eng);
		CHECK(ps, application/postscript);
		CHECK(psd, application/octet-stream);
		CHECK(pvu, paleovu/x-pv);
		CHECK(pwz, application/vnd.ms-powerpoint);
		CHECK(py, text/x-script.phyton);
		CHECK(pyc, applicaiton/x-bytecode.python);
		break;
	case 'q':
		CHECK(qcp, audio/vnd.qcelp);
		CHECK(qd3, x-world/x-3dmf);
		CHECK(qd3d, x-world/x-3dmf);
		CHECK(qif, image/x-quicktime);
		CHECK(qt, video/quicktime);
		CHECK(qtc, video/x-qtc);
		CHECK(qti, image/x-quicktime);
		CHECK(qtif, image/x-quicktime);
		break;
	case 'r':
		CHECK(ra, audio/x-pn-realaudio);
		CHECK(ram, audio/x-pn-realaudio);
		CHECK(ras, application/x-cmu-raster);
		CHECK(rast, image/cmu-raster);
		CHECK(rexx, text/x-script.rexx);
		CHECK(rf, image/vnd.rn-realflash);
		CHECK(rgb, image/x-rgb);
		CHECK(rm, application/vnd.rn-realmedia);
		CHECK(rmi, audio/mid);
		CHECK(rmm, audio/x-pn-realaudio);
		CHECK(rmp, audio/x-pn-realaudio);
		CHECK(rng, application/ringing-tones);
		CHECK(rnx, application/vnd.rn-realplayer);
		CHECK(roff, application/x-troff);
		CHECK(rp, image/vnd.rn-realpix);
		CHECK(rpm, audio/x-pn-realaudio-plugin);
		CHECK(rt, text/richtext);
		CHECK(rtf, application/rtf);
		CHECK(rtx, application/rtf);
		CHECK(rv, video/vnd.rn-realvideo);
		break;
	case 's':
		CHECK(s, text/x-asm);
		CHECK(s3m, audio/s3m);
		CHECK(saveme, application/octet-stream);
		CHECK(sbk, application/x-tbook);
		CHECK(scm, text/x-script.scheme);
		CHECK(sdml, text/plain);
		CHECK(sdp, application/sdp);
		CHECK(sdr, application/sounder);
		CHECK(sea, application/sea);
		CHECK(set, application/set);
		CHECK(sgm, text/sgml);
		CHECK(sgml, text/sgml);
		CHECK(sh, application/x-sh);
		CHECK(shar, application/x-bsh);
		CHECK(shtml, text/html);
		CHECK(sid, audio/x-psid);
		CHECK(sit, application/x-sit);
		CHECK(skd, application/x-koan);
		CHECK(skm, application/x-koan);
		CHECK(skp, application/x-koan);
		CHECK(skt, application/x-koan);
		CHECK(sl, application/x-seelogo);
		CHECK(smi, application/smil);
		CHECK(smil, application/smil);
		CHECK(snd, audio/basic);
		CHECK(sol, application/solids);
		CHECK(spc, application/x-pkcs7-certificates);
		CHECK(spl, application/futuresplash);
		CHECK(spr, application/x-sprite);
		CHECK(sprite, application/x-sprite);
		CHECK(src, application/x-wais-source);
		CHECK(ssi, text/x-server-parsed-html);
		CHECK(ssm, application/streamingmedia);
		CHECK(sst, application/vnd.ms-pki.certstore);
		CHECK(step, application/step);
		CHECK(stl, application/sla);
		CHECK(stp, application/step);
		CHECK(sv4cpio, application/x-sv4cpio);
		CHECK(sv4crc, application/x-sv4crc);
		CHECK(svf, image/x-dwg);
		CHECK(svr, application/x-world);
		CHECK(swf, application/x-shockwave-flash);
		break;
	case 't':
		CHECK(t, application/x-troff);
		CHECK(talk, text/x-speech);
		CHECK(tar, application/x-tar);
		CHECK(tbk, application/toolbook);
		CHECK(tcl, application/x-tcl);
		CHECK(tcsh, text/x-script.tcsh);
		CHECK(tex, application/x-tex);
		CHECK(texi, application/x-texinfo);
		CHECK(texinfo, application/x-texinfo);
		CHECK(text, text/plain);
		CHECK(tgz, application/gnutar);
		CHECK(tgz, application/x-compressed);
		CHECK(tif, image/tiff);
		CHECK(tiff, image/tiff);
		CHECK(tr, application/x-troff);
		CHECK(tsi, audio/tsp-audio);
		CHECK(tsp, application/dsptype);
		CHECK(tsv, text/tab-separated-values);
		CHECK(turbot, image/florian);
		CHECK(txt, text/plain);
		break;
	case 'u':
		CHECK(uil, text/x-uil);
		CHECK(uni, text/uri-list);
		CHECK(unis, text/uri-list);
		CHECK(unv, application/i-deas);
		CHECK(uri, text/uri-list);
		CHECK(uris, text/uri-list);
		CHECK(ustar, application/x-ustar);
		CHECK(ustar, multipart/x-ustar);
		CHECK(uu, application/octet-stream);
		CHECK(uue, text/x-uuencode);
		break;
	case 'v':
		CHECK(vcd, application/x-cdlink);
		CHECK(vcs, text/x-vcalendar);
		CHECK(vda, application/vda);
		CHECK(vdo, video/vdo);
		CHECK(vew, application/groupwise);
		CHECK(viv, video/vivo);
		CHECK(vivo, video/vivo);
		CHECK(vmd, application/vocaltec-media-desc);
		CHECK(vmf, application/vocaltec-media-file);
		CHECK(voc, audio/voc);
		CHECK(vos, video/vosaic);
		CHECK(vox, audio/voxware);
		CHECK(vqe, audio/x-twinvq-plugin);
		CHECK(vqf, audio/x-twinvq);
		CHECK(vql, audio/x-twinvq-plugin);
		CHECK(vrml, application/x-vrml);
		CHECK(vrt, x-world/x-vrt);
		CHECK(vsd, application/x-visio);
		CHECK(vst, application/x-visio);
		CHECK(vsw, application/x-visio);
		break;
	case 'w':
		CHECK(w60, application/wordperfect6.0);
		CHECK(w61, application/wordperfect6.1);
		CHECK(w6w, application/msword);
		CHECK(wav, audio/wav);
		CHECK(wb1, application/x-qpro);
		CHECK(wbmp, image/vnd.wap.wbmp);
		CHECK(web, application/vnd.xara);
		CHECK(wiz, application/msword);
		CHECK(wk1, application/x-123);
		CHECK(wmf, windows/metafile);
		CHECK(wml, text/vnd.wap.wml);
		CHECK(wmlc, application/vnd.wap.wmlc);
		CHECK(wmls, text/vnd.wap.wmlscript);
		CHECK(wmlsc, application/vnd.wap.wmlscriptc);
		CHECK(word, application/msword);
		CHECK(wp, application/wordperfect);
		CHECK(wp5, application/wordperfect);
		CHECK(wp6, application/wordperfect);
		CHECK(wpd, application/wordperfect);
		CHECK(wpd, application/x-wpwin);
		CHECK(wq1, application/x-lotus);
		CHECK(wri, application/mswrite);
		CHECK(wrl, application/x-world);
		CHECK(wrz, model/vrml);
		CHECK(wsc, text/scriplet);
		CHECK(wsrc, application/x-wais-source);
		CHECK(wtk, application/x-wintalk);
		break;
	case 'x':
		CHECK(xbm, image/x-xbitmap);
		CHECK(xdr, video/x-amt-demorun);
		CHECK(xgz, xgl/drawing);
		CHECK(xif, image/vnd.xiff);
		CHECK(xl, application/excel);
		CHECK(xla, application/excel);
		CHECK(xlb, application/excel);
		CHECK(xlc, application/excel);
		CHECK(xld, application/excel);
		CHECK(xlk, application/excel);
		CHECK(xll, application/excel);
		CHECK(xlm, application/excel);
		CHECK(xls, application/excel);
		CHECK(xlt, application/excel);
		CHECK(xlv, application/excel);
		CHECK(xlw, application/excel);
		CHECK(xm, audio/xm);
		CHECK(xml, application/xml);
		CHECK(xml, text/xml);
		CHECK(xmz, xgl/movie);
		CHECK(xpix, application/x-vnd.ls-xpix);
		CHECK(xpm, image/xpm);
		CHECK(x-png, image/png);
		CHECK(xsr, video/x-amt-showrun);
		CHECK(xwd, image/x-xwd);
		CHECK(xyz, chemical/x-pdb);
		break;
	case 'z':
		CHECK(z, application/x-compress);
		CHECK(zip, application/x-compressed);
		CHECK(zoo, application/octet-stream);
		CHECK(zsh, text/x-script.zsh);
		break;
	}

	return "application/octet-stream";
}
